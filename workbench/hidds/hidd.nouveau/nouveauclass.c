/*
    Copyright � 2010, The AROS Development Team. All rights reserved.
    $Id$
*/

#include "nouveau_intern.h"
#include "nouveau_compositing.h"
#include "nouveau/nouveau_class.h"

#include <graphics/displayinfo.h>
#include <proto/utility.h>

#define DEBUG 0
#include <aros/debug.h>
#include <proto/oop.h>

#include "arosdrmmode.h"

#undef HiddPixFmtAttrBase
#undef HiddGfxAttrBase
#undef HiddSyncAttrBase
#undef HiddBitMapAttrBase
#define HiddPixFmtAttrBase  (SD(cl)->pixFmtAttrBase)
#define HiddGfxAttrBase     (SD(cl)->gfxAttrBase)
#define HiddSyncAttrBase    (SD(cl)->syncAttrBase)
#define HiddBitMapAttrBase  (SD(cl)->bitMapAttrBase)

/* HELPER FUNCTIONS */
VOID HIDDNouveauShowCursor(OOP_Object * gfx, BOOL visible)
{
    OOP_Class * cl = OOP_OCLASS(gfx);
    struct HIDDNouveauData * gfxdata = OOP_INST_DATA(cl, gfx);
    struct CardData * carddata = &(SD(cl)->carddata);
    struct nouveau_device_priv * nvdev = nouveau_device(carddata->dev);

    if (visible)
    {
        drmModeSetCursor(nvdev->fd, gfxdata->selectedcrtcid, 
            gfxdata->cursor->handle, 64, 64);
    }
    else
    {
        drmModeSetCursor(nvdev->fd, gfxdata->selectedcrtcid, 
            0, 64, 64);
    }
}

static BOOL HIDDNouveauSelectConnectorCrtc(LONG fd, drmModeConnectorPtr * selectedconnector,
    drmModeCrtcPtr * selectedcrtc)
{
    *selectedconnector = NULL;
    *selectedcrtc = NULL;
    drmModeResPtr drmmode = NULL;
    drmModeEncoderPtr selectedencoder = NULL;
    LONG i; ULONG crtc_id;

    /* Get all components information */
    drmmode = drmModeGetResources(fd);
    if (!drmmode)
    {
        D(bug("[Nouveau] Not able to get resources information\n"));
        return FALSE;
    }
    
    /* Selecting connector */
    for (i = 0; i < drmmode->count_connectors; i++)
    {
        drmModeConnectorPtr connector = drmModeGetConnector(fd, drmmode->connectors[i]);

        if (connector)
        {
            if (connector->connection == DRM_MODE_CONNECTED)
            {
                /* Found connected connector */
                *selectedconnector = connector;
                break;
            }
            
            drmModeFreeConnector(connector);
        }
    }
    
    if (!(*selectedconnector))
    {
        D(bug("[Nouveau] No connected connector\n"));
        drmModeFreeResources(drmmode);
        return FALSE;
    }

    /* Selecting crtc (from encoder) */
    selectedencoder = drmModeGetEncoder(fd, (*selectedconnector)->encoder_id);
    
    if (!selectedencoder)
    {
        D(bug("[Nouveau] Not able to get encoder information for enc_id %d\n", (*selectedconnector)->encoder_id));
        drmModeFreeConnector(*selectedconnector);
        *selectedconnector = NULL;
        drmModeFreeResources(drmmode);
        return FALSE;
    }

    /* WARNING: CRTC_ID from encoder seems to be zero after first mode switch */
    crtc_id = selectedencoder->crtc_id;
    drmModeFreeEncoder(selectedencoder);

    *selectedcrtc = drmModeGetCrtc(fd, crtc_id);
    if (!(*selectedcrtc))
    {
        D(bug("[Nouveau] Not able to get crtc information for crtc_id %d\n", crtc_id));
        drmModeFreeConnector(*selectedconnector);
        *selectedconnector = NULL;
        drmModeFreeResources(drmmode);
        return FALSE;
    }
    
    drmModeFreeResources(drmmode);
    return TRUE;
}    

#include <stdio.h>

static struct TagItem * HIDDNouveauCreateSyncTagsFromConnector(OOP_Class * cl, drmModeConnectorPtr connector)
{
    struct TagItem * syncs = NULL;
    ULONG modescount = connector->count_modes;
    ULONG i;
    
    if (modescount == 0)
        return NULL;
        
    /* Allocate enough structures */
    syncs = AllocVec(sizeof(struct TagItem) * modescount, MEMF_ANY);
    
    for (i = 0; i < modescount; i++)
    {
        struct TagItem * sync = AllocVec(sizeof(struct TagItem) * 15, MEMF_ANY);
        LONG j = 0;
        
        drmModeModeInfoPtr mode = &connector->modes[i];

        sync[j].ti_Tag = aHidd_Sync_PixelClock;     sync[j++].ti_Data = mode->clock;

        sync[j].ti_Tag = aHidd_Sync_HDisp;          sync[j++].ti_Data = mode->hdisplay;
        sync[j].ti_Tag = aHidd_Sync_HSyncStart;     sync[j++].ti_Data = mode->hsync_start;
        sync[j].ti_Tag = aHidd_Sync_HSyncEnd;       sync[j++].ti_Data = mode->hsync_end;
        sync[j].ti_Tag = aHidd_Sync_HTotal;         sync[j++].ti_Data = mode->htotal;
        sync[j].ti_Tag = aHidd_Sync_HMin;           sync[j++].ti_Data = mode->hdisplay;
        sync[j].ti_Tag = aHidd_Sync_HMax;           sync[j++].ti_Data = 2048;

        sync[j].ti_Tag = aHidd_Sync_VDisp;          sync[j++].ti_Data = mode->vdisplay;
        sync[j].ti_Tag = aHidd_Sync_VSyncStart;     sync[j++].ti_Data = mode->vsync_start;
        sync[j].ti_Tag = aHidd_Sync_VSyncEnd;       sync[j++].ti_Data = mode->vsync_end;
        sync[j].ti_Tag = aHidd_Sync_VTotal;         sync[j++].ti_Data = mode->vtotal;
        sync[j].ti_Tag = aHidd_Sync_VMin;           sync[j++].ti_Data = mode->vdisplay;
        sync[j].ti_Tag = aHidd_Sync_VMax;           sync[j++].ti_Data = 2048;
        
        /* Name */
        STRPTR syncname = AllocVec(32, MEMF_ANY | MEMF_CLEAR);
        sprintf(syncname, "NV:%dx%d@%d", mode->hdisplay, mode->vdisplay, mode->vrefresh);
        
        sync[j].ti_Tag = aHidd_Sync_Description;   sync[j++].ti_Data = (IPTR)syncname;
        
        sync[j].ti_Tag = TAG_DONE;                 sync[j++].ti_Data = 0UL;
        
        syncs[i].ti_Tag = aHidd_Gfx_SyncTags;
        syncs[i].ti_Data = (IPTR)sync;
    }
    
    return syncs;
}

/* PUBLIC METHODS */
OOP_Object * METHOD(Nouveau, Root, New)
{
    drmModeCrtcPtr selectedcrtc = NULL;
    drmModeConnectorPtr selectedconnector = NULL;
    struct nouveau_device * dev = NULL;
    struct nouveau_device_priv * nvdev = NULL;
    struct TagItem * syncs = NULL;
    struct CardData * carddata = &(SD(cl)->carddata);
    LONG ret;
    ULONG selectedcrtcid;
    
    nouveau_init();

    nouveau_device_open(&dev, "");
    nvdev = nouveau_device(dev);

    /* Select crtc and connector */
    if (!HIDDNouveauSelectConnectorCrtc(nvdev->fd, &selectedconnector, &selectedcrtc))
    {
        D(bug("[Nouveau] Not able to select connector and crtc\n"));
        return NULL;
    }
    
    selectedcrtcid = selectedcrtc->crtc_id;
    drmModeFreeCrtc(selectedcrtc);

    /* Read connector and build sync tags */
    syncs = HIDDNouveauCreateSyncTagsFromConnector(cl, selectedconnector);
    if (syncs == NULL)
    {
        D(bug("[Nouveau] Not able to read any sync modes\n"));
        return NULL;
    }
    

    /* Call super contructor */
    {
        struct TagItem pftags_24bpp[] = {
        { aHidd_PixFmt_RedShift,	8	}, /* 0 */
        { aHidd_PixFmt_GreenShift,	16	}, /* 1 */
        { aHidd_PixFmt_BlueShift,  	24	}, /* 2 */
        { aHidd_PixFmt_AlphaShift,	0	}, /* 3 */
        { aHidd_PixFmt_RedMask,		0x00ff0000 }, /* 4 */
        { aHidd_PixFmt_GreenMask,	0x0000ff00 }, /* 5 */
        { aHidd_PixFmt_BlueMask,	0x000000ff }, /* 6 */
        { aHidd_PixFmt_AlphaMask,	0x00000000 }, /* 7 */
        { aHidd_PixFmt_ColorModel,	vHidd_ColorModel_TrueColor }, /* 8 */
        { aHidd_PixFmt_Depth,		24	}, /* 9 */
        { aHidd_PixFmt_BytesPerPixel,	4	}, /* 10 */
        { aHidd_PixFmt_BitsPerPixel,	24	}, /* 11 */
        { aHidd_PixFmt_StdPixFmt,	vHidd_StdPixFmt_BGR032 }, /* 12 Native */
        { aHidd_PixFmt_BitMapType,	vHidd_BitMapType_Chunky }, /* 15 */
        { TAG_DONE, 0UL }
        };

        struct TagItem pftags_16bpp[] = {
        { aHidd_PixFmt_RedShift,	16	}, /* 0 */
        { aHidd_PixFmt_GreenShift,	21	}, /* 1 */
        { aHidd_PixFmt_BlueShift,  	27	}, /* 2 */
        { aHidd_PixFmt_AlphaShift,	0	}, /* 3 */
        { aHidd_PixFmt_RedMask,		0x0000f800 }, /* 4 */
        { aHidd_PixFmt_GreenMask,	0x000007e0 }, /* 5 */
        { aHidd_PixFmt_BlueMask,	0x0000001f }, /* 6 */
        { aHidd_PixFmt_AlphaMask,	0x00000000 }, /* 7 */
        { aHidd_PixFmt_ColorModel,	vHidd_ColorModel_TrueColor }, /* 8 */
        { aHidd_PixFmt_Depth,		16	}, /* 9 */
        { aHidd_PixFmt_BytesPerPixel,	2	}, /* 10 */
        { aHidd_PixFmt_BitsPerPixel,	16	}, /* 11 */
        { aHidd_PixFmt_StdPixFmt,	vHidd_StdPixFmt_RGB16_LE }, /* 12 */
        { aHidd_PixFmt_BitMapType,	vHidd_BitMapType_Chunky }, /* 15 */
        { TAG_DONE, 0UL }
        };

        struct TagItem modetags[] = {
	    { aHidd_Gfx_PixFmtTags,	(IPTR)pftags_24bpp	},
	    { aHidd_Gfx_PixFmtTags,	(IPTR)pftags_16bpp	},
        { TAG_MORE, (IPTR)syncs },  /* FIXME: sync tags will leak */
	    { TAG_DONE, 0UL }
        };

        struct TagItem mytags[] = {
	    { aHidd_Gfx_ModeTags,	(IPTR)modetags	},
	    { TAG_MORE, (IPTR)msg->attrList }
        };

        struct pRoot_New mymsg;

        mymsg.mID = msg->mID;
        mymsg.attrList = mytags;

        msg = &mymsg;


        o = (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);

        D(bug("Nouveau::New\n"));

        if (o)
        {
            struct HIDDNouveauData * gfxdata = OOP_INST_DATA(cl, o);
            /* Pass local information to class */
            gfxdata->selectedcrtcid = selectedcrtcid;
            gfxdata->selectedmode = NULL;
            gfxdata->selectedconnector = selectedconnector;
            carddata->dev = dev;
            ULONG gartsize = 0;
            
            /* Check chipset architecture */
            switch (carddata->dev->chipset & 0xf0) 
            {
            case 0x00:
                carddata->architecture = NV_ARCH_04;
                break;
            case 0x10:
                carddata->architecture = NV_ARCH_10;
                break;
            case 0x20:
                carddata->architecture = NV_ARCH_20;
                break;
            case 0x30:
                carddata->architecture = NV_ARCH_30;
                break;
            case 0x40:
            case 0x60:
                carddata->architecture = NV_ARCH_40;
                break;
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
                carddata->architecture = NV_ARCH_50;
                break;
            default:
                /* TODO: report error, how to handle it? */
                return NULL;
            }

            /* Allocate dma channel */
            ret = nouveau_channel_alloc(carddata->dev, NvDmaFB, NvDmaTT, &carddata->chan);
            /* TODO: Check ret, how to handle ? */

            /* Initialize acceleration objects */
            ret = NVAccelCommonInit(carddata);
            /* TODO: Check ret, how to handle ? */

            /* Allocate buffer object for cursor */
            nouveau_bo_new(carddata->dev, NOUVEAU_BO_VRAM | NOUVEAU_BO_MAP, 
                0, 64 * 64 * 4, &gfxdata->cursor);
            /* TODO: Check return, hot to handle */
            
            /* Allocate GART scratch buffer */
            if (carddata->dev->vm_gart_size > (16 * 1024 * 1024))
                gartsize = 16 * 1024 * 1024;
            else
                /* always leave 512kb for other things like the fifos */
                gartsize = carddata->dev->vm_gart_size - 512*1024;

            /* This can fail */
            nouveau_bo_new(carddata->dev, NOUVEAU_BO_GART | NOUVEAU_BO_MAP,
                0, gartsize, &carddata->GART);
            InitSemaphore(&carddata->gartsemaphore);
            
            /* Set initial pattern (else 16-bit ROPs are not working) */
            if (carddata->architecture == NV_ARCH_50)
                HIDDNouveauNV50SetPattern(carddata, ~0, ~0, ~0, ~0);
            else
                HIDDNouveauNV04SetPattern(carddata, ~0, ~0, ~0, ~0);
        }

        return o;
    }
    
    return NULL;
}

/* FIXME: IMPLEMENT DISPOSE - calling nouveau_close(), freeing cursor bo, gart bo, selectedconnector */

/* FIXME: IMPLEMENT DISPOSE BITMAP - REMOVE FROM FB IF MARKED AS SUCH */

OOP_Object * METHOD(Nouveau, Hidd_Gfx, NewBitMap)
{
    struct pHidd_Gfx_NewBitMap mymsg;
    HIDDT_ModeID modeid;
    HIDDT_StdPixFmt stdpf;
    struct TagItem mytags [] =
    {
        { TAG_IGNORE, TAG_IGNORE }, /* Placeholder for aHidd_BitMap_ClassPtr */
        { TAG_IGNORE, TAG_IGNORE }, /* Placeholder for aHidd_BitMap_Align */
        { TAG_MORE, (IPTR)msg->attrList }
    };

    /* Check if user provided valid ModeID */
    /* Check for framebuffer - not needed as Nouveau is a NoFramebuffer driver */
    /* Check for displayable - not needed - displayable has ModeID and we don't
       distinguish between on-screen and off-screen bitmaps */
    modeid = (HIDDT_ModeID)GetTagData(aHidd_BitMap_ModeID, vHidd_ModeID_Invalid, msg->attrList);
    if (vHidd_ModeID_Invalid != modeid) 
    {
        /* User supplied a valid modeid. We can use our bitmap class */
        mytags[0].ti_Tag	= aHidd_BitMap_ClassPtr;
        mytags[0].ti_Data	= (IPTR)SD(cl)->bmclass;
    } 

    /* Check if bitmap is a planar bitmap */
    stdpf = (HIDDT_StdPixFmt)GetTagData(aHidd_BitMap_StdPixFmt, vHidd_StdPixFmt_Unknown, msg->attrList);
    if (vHidd_StdPixFmt_Plane == stdpf)
    {
        mytags[1].ti_Tag    = aHidd_BitMap_Align;
        mytags[1].ti_Data   = 32;
    }
    
    /* We init a new message struct */
    mymsg.mID	= msg->mID;
    mymsg.attrList	= mytags;

    /* Pass the new message to the superclass */
    msg = &mymsg;

    return (OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
}

#define IS_NOUVEAU_CLASS(x) (x == SD(cl)->bmclass)

VOID METHOD(Nouveau, Hidd_Gfx, CopyBox)
{
    OOP_Class * srcclass = OOP_OCLASS(msg->src);
    OOP_Class * destclass = OOP_OCLASS(msg->dest);
    
    if (IS_NOUVEAU_CLASS(srcclass) && IS_NOUVEAU_CLASS(destclass))
    {
        /* FIXME: add checks for pixel format, etc */
        struct HIDDNouveauBitMapData * srcdata = OOP_INST_DATA(srcclass, msg->src);
        struct HIDDNouveauBitMapData * destdata = OOP_INST_DATA(destclass, msg->dest);
        struct CardData * carddata = &(SD(cl)->carddata);
        BOOL ret = FALSE;
        
        LOCK_MULTI_BITMAP
        LOCK_BITMAP_BM(srcdata)
        LOCK_BITMAP_BM(destdata)
        UNLOCK_MULTI_BITMAP
        UNMAP_BUFFER_BM(srcdata)
        UNMAP_BUFFER_BM(destdata)
        
        if (carddata->architecture < NV_ARCH_50)
        {
            ret = HIDDNouveauNV04CopySameFormat(carddata, srcdata, destdata, 
                        msg->srcX, msg->srcY, msg->destX, msg->destY, 
                        msg->width, msg->height, GC_DRMD(msg->gc));
        }
        else
        {
            ret = HIDDNouveauNV50CopySameFormat(carddata, srcdata, destdata, 
                        msg->srcX, msg->srcY, msg->destX, msg->destY, 
                        msg->width, msg->height, GC_DRMD(msg->gc));
        }

        UNLOCK_BITMAP_BM(destdata);
        UNLOCK_BITMAP_BM(srcdata);

        if (ret)
            return;
        
        /* If operation failed, fallback to default method */
    }

    OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    
}

VOID METHOD(Nouveau, Root, Get)
{
    ULONG idx;

    if (IS_GFX_ATTR(msg->attrID, idx))
    {
        switch (idx)
        {
        case aoHidd_Gfx_NoFrameBuffer:
            *msg->storage = (IPTR)TRUE;
            return;
        case aoHidd_Gfx_SupportsHWCursor:
            *msg->storage = (IPTR)TRUE;
            return;
        case aoHidd_Gfx_HWSpriteTypes:
            *msg->storage = vHidd_SpriteType_DirectColor;
            return;
	    case aoHidd_Gfx_DriverName:
    		*msg->storage = (IPTR)"Nouveau";
    		return;
        }
    }

    OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
}

ULONG METHOD(Nouveau, Hidd_Gfx, ShowViewPorts)
{
    D(bug("[Nouveau] ShowViewPorts enter TopLevelBM %x\n", msg->Data->Bitmap));
#if ENABLE_COMPOSITING
    /* TODO: probably needs a driver level lock? */
    Compositing_BitMapStackChanged(msg->Data);
    
#else
    if (msg->Data)
        Compositing_TopBitMapChanged(msg->Data->Bitmap);
    else
    {
        /* TODO: Implement blanking out display */
    }    
#endif
    return TRUE; /* Indicate driver supports this method */
}

#if AROS_BIG_ENDIAN
#define Machine_ARGB32 vHidd_StdPixFmt_ARGB32
#else
#define Machine_ARGB32 vHidd_StdPixFmt_BGRA32
#endif

BOOL METHOD(Nouveau, Hidd_Gfx, SetCursorShape)
{
    struct HIDDNouveauData * gfxdata = OOP_INST_DATA(cl, o);
        
    if (msg->shape == NULL)
    {
        /* Hide cursor */
        HIDDNouveauShowCursor(o, FALSE);
    }
    else
    {
        IPTR width, height;
        ULONG i;
        ULONG x, y;
        ULONG curimage[64 * 64];
        struct CardData * carddata = &(SD(cl)->carddata);
        
        OOP_GetAttr(msg->shape, aHidd_BitMap_Width, &width);
        OOP_GetAttr(msg->shape, aHidd_BitMap_Height, &height);


        if (width > 64) width = 64;
        if (height > 64) height = 64;

        /* Map the cursor buffer */
        nouveau_bo_map(gfxdata->cursor, NOUVEAU_BO_WR);

        /* Clear the matrix */
        for (i = 0; i < 64 * 64; i++)
            ((ULONG*)gfxdata->cursor->map)[i] = 0;

        /* Get data from the bitmap */
        HIDD_BM_GetImage(msg->shape, (UBYTE *)curimage, 64 * 4, 0, 0, 
            width, height, Machine_ARGB32);
        
        if (carddata->architecture < NV_ARCH_50)
        {
            ULONG offset, pixel, blue, green, red, alpha;

            /* The image needs to be premultiplied */
            for (y = 0; y < height; y++)
                for (x = 0; x < width; x++)
                {
                    offset = y * 64 + x;
                    pixel = curimage[offset];
                    blue  = (pixel & 0x000000FF);
                    green = (pixel & 0x0000FF00) >> 8;
                    red   = (pixel & 0x00FF0000) >> 16;
                    alpha = (pixel & 0xFF000000) >> 24;
                    
                    blue    = (blue * alpha) / 255;
                    green   = (green * alpha) / 255;
                    red     = (red * alpha) / 255;

                    curimage[offset]    = (alpha << 24) | (red << 16) | (green << 8) | blue;
                }
        }

        for (y = 0; y < height; y++)
            for (x = 0; x < width; x++)
            {
                ULONG offset = y * 64 + x;
                writel(curimage[offset], ((ULONG *)gfxdata->cursor->map) + (offset));
            }

        nouveau_bo_unmap(gfxdata->cursor);
        
        /* Show updated cursor */
        HIDDNouveauShowCursor(o, TRUE);
    }

    return TRUE;   
}

BOOL METHOD(Nouveau, Hidd_Gfx, SetCursorPos)
{
    struct HIDDNouveauData * gfxdata = OOP_INST_DATA(cl, o);
    struct CardData * carddata = &(SD(cl)->carddata);
    struct nouveau_device_priv * nvdev = nouveau_device(carddata->dev);

    drmModeMoveCursor(nvdev->fd, gfxdata->selectedcrtcid, msg->x, msg->y);
    
    return TRUE;
}

VOID METHOD(Nouveau, Hidd_Gfx, SetCursorVisible)
{
    HIDDNouveauShowCursor(o, msg->visible);
}

static struct HIDD_ModeProperties modeprops = 
{
    DIPF_IS_SPRITES,
    1,
#if ENABLE_COMPOSITING
    COMPF_ABOVE
#else
    0
#endif
};

ULONG METHOD(Nouveau, Hidd_Gfx, ModeProperties)
{
    ULONG len = msg->propsLen;

    if (len > sizeof(modeprops))
        len = sizeof(modeprops);
    CopyMem(&modeprops, msg->props, len);

    return len;
}


