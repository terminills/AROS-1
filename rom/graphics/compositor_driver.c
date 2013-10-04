#include <aros/debug.h>
#include <graphics/driver.h>
#include <hidd/compositor.h>
#include <oop/oop.h>

#include <proto/oop.h>

#include "graphics_intern.h"
#include "compositor_driver.h"

ULONG compositor_Install(OOP_Class *cl, struct GfxBase *GfxBase)
{
    struct monitor_driverdata *mdd;

    D(bug("[Compositor] Installing class 0x%p\n", cl));

    /*
     * Completely lazy initialization.
     * It even can't be done in other way because OOP_GetMethodID() on an unregistered
     * interface will fail (unlike OOP_ObtainAttrBase).
     */
    if (!HiddCompositorAttrBase)
    	HiddCompositorAttrBase = OOP_ObtainAttrBase(IID_Hidd_Compositor);

    if (!HiddCompositorAttrBase)
    	return DD_NO_MEM;

    if (!PrivGBase(GfxBase)->HiddCompositorMethodBase)
    {
    	PrivGBase(GfxBase)->HiddCompositorMethodBase = OOP_GetMethodID(IID_Hidd_Compositor, 0);

	if (!PrivGBase(GfxBase)->HiddCompositorMethodBase)
	{
    	    OOP_ReleaseAttrBase(IID_Hidd_Compositor);

    	    return DD_NO_MEM;
    	}
    }

    CDD(GfxBase)->compositorClass = cl;

    for (mdd = CDD(GfxBase)->monitors; mdd; mdd = mdd->next)
    {
	/* If the driver needs software compositor, but has no one, instantiate it. */
    	if ((mdd->flags & DF_SoftCompose) && (!mdd->compositor))
    	{
    	    compositor_Setup(mdd, GfxBase);
    	}
    }

    return DD_OK;
}

void compositor_Setup(struct monitor_driverdata *mdd, struct GfxBase *GfxBase)
{
    /* 
     * Note that if we have fakegfx object, we'll actually work on top of it.
     * This allows us to have transparent software mouse pointer support.
     */
    mdd->compositor = OOP_NewObjectTags(CDD(GfxBase)->compositorClass, NULL,
				      aHidd_Compositor_GfxHidd, mdd->gfxhidd, 
				      aHidd_Compositor_FrameBuffer, mdd->framebuffer, TAG_DONE);

    /* ... but print name of the original driver, to be informative */
    D(bug("[Compositor] Added compositor object 0x%p to driver 0x%p (%s)\n", mdd->compositor, mdd->gfxhidd,
     	  OOP_OCLASS(mdd->gfxhidd_orig)->ClassNode.ln_Name));
}

BOOL compositor_IsBMCompositable(OOP_Object *bm, struct GfxBase *GfxBase)
{
    D(bug("[GfxCompositor] IsBm @ 0x%p Compositable?\n", bm));

    return TRUE;
}

BOOL compositor_SetBMCompositable(OOP_Object *bm, struct GfxBase *GfxBase)
{
    if (!(OOP_GET(bm, aHidd_BitMap_Displayable)))
    {
        struct TagItem composittags[] = {
            {aHidd_BitMap_Compositable, TRUE},
            {TAG_DONE	     , 0    }
        };

        D(bug("[GfxCompositor]: Marking BitMap 0x%lx as Compositable\n", bm));
        OOP_SetAttrs(bm, composittags);
    }
    return TRUE;
}