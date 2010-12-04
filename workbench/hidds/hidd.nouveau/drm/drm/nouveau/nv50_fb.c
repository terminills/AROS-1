#include "drmP.h"
#include "drm.h"
#include "nouveau_drv.h"
#include "nouveau_drm.h"

struct nv50_fb_priv {
	struct page *r100c08_page;
	dma_addr_t r100c08;
};

static int
nv50_fb_create(struct drm_device *dev)
{
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	struct nv50_fb_priv *priv;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

#if !defined(__AROS__)
	priv->r100c08_page = alloc_page(GFP_KERNEL | __GFP_ZERO);
#else
	priv->r100c08_page = create_page_helper();
#endif
	if (!priv->r100c08_page) {
		kfree(priv);
		return -ENOMEM;
	}

	priv->r100c08 = pci_map_page(dev->pdev, priv->r100c08_page, 0,
				     PAGE_SIZE, PCI_DMA_BIDIRECTIONAL);
	if (pci_dma_mapping_error(dev->pdev, priv->r100c08)) {
		__free_page(priv->r100c08_page);
		kfree(priv);
		return -EFAULT;
	}

	dev_priv->engine.fb.priv = priv;
	return 0;
}

int
nv50_fb_init(struct drm_device *dev)
{
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	struct nv50_fb_priv *priv;
	int ret;

	if (!dev_priv->engine.fb.priv) {
		ret = nv50_fb_create(dev);
		if (ret)
			return ret;
	}
	priv = dev_priv->engine.fb.priv;

	/* Not a clue what this is exactly.  Without pointing it at a
	 * scratch page, VRAM->GART blits with M2MF (as in DDX DFS)
	 * cause IOMMU "read from address 0" errors (rh#561267)
	 */
	nv_wr32(dev, 0x100c08, priv->r100c08 >> 8);

	/* This is needed to get meaningful information from 100c90
	 * on traps. No idea what these values mean exactly. */
	switch (dev_priv->chipset) {
	case 0x50:
		nv_wr32(dev, 0x100c90, 0x000707ff);
		break;
	case 0xa3:
	case 0xa5:
	case 0xa8:
		nv_wr32(dev, 0x100c90, 0x000d0fff);
		break;
	case 0xaf:
		nv_wr32(dev, 0x100c90, 0x089d1fff);
		break;
	default:
		nv_wr32(dev, 0x100c90, 0x001d07ff);
		break;
	}

	return 0;
}

void
nv50_fb_takedown(struct drm_device *dev)
{
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	struct nv50_fb_priv *priv;

	priv = dev_priv->engine.fb.priv;
	if (!priv)
		return;
	dev_priv->engine.fb.priv = NULL;

	pci_unmap_page(dev->pdev, priv->r100c08, PAGE_SIZE,
		       PCI_DMA_BIDIRECTIONAL);
	__free_page(priv->r100c08_page);
	kfree(priv);
}

void
nv50_fb_vm_trap(struct drm_device *dev, int display, const char *name)
{
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	unsigned long flags;
	u32 trap[6], idx, chinst;
	int i, ch;

	idx = nv_rd32(dev, 0x100c90);
	if (!(idx & 0x80000000))
		return;
	idx &= 0x00ffffff;

	for (i = 0; i < 6; i++) {
		nv_wr32(dev, 0x100c90, idx | i << 24);
		trap[i] = nv_rd32(dev, 0x100c94);
	}
	nv_wr32(dev, 0x100c90, idx | 0x80000000);

	if (!display)
		return;

	chinst = (trap[2] << 16) | trap[1];

	spin_lock_irqsave(&dev_priv->channels.lock, flags);
	for (ch = 0; ch < dev_priv->engine.fifo.channels; ch++) {
		struct nouveau_channel *chan = dev_priv->channels.ptr[ch];

		if (!chan || !chan->ramin)
			continue;

		if (chinst == chan->ramin->vinst >> 12)
			break;
	}
	spin_unlock_irqrestore(&dev_priv->channels.lock, flags);

	NV_INFO(dev, "%s - VM: Trapped %s at %02x%04x%04x status %08x "
		     "channel %d (0x%08x)\n",
		name, (trap[5] & 0x100 ? "read" : "write"),
		trap[5] & 0xff, trap[4] & 0xffff, trap[3] & 0xffff,
		trap[0], ch, chinst);
}
