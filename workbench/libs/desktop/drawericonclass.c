
#define DEBUG 1
#include <aros/debug.h>

#include <exec/types.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <libraries/mui.h>

#include "support.h"
#include "worker.h"
#include "desktop_intern.h"

#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include "presentation.h"
#include "drawericonclass.h"

#include "desktop_intern_protos.h"

IPTR drawerIconNew(Class * cl, Object * obj, struct opSet * msg)
{
    IPTR            retval = 0;
    struct DrawerIconClassData *data;
    struct TagItem *tag;

    retval = DoSuperMethodA(cl, obj, (Msg) msg);
    if (retval)
    {
        obj = (Object *) retval;
        data = INST_DATA(cl, obj);
    }

    return retval;
}

IPTR drawerIconSet(Class * cl, Object * obj, struct opSet * msg)
{
    struct DrawerIconClassData *data;
    IPTR            retval = 1;
    struct TagItem *tag,
                   *tstate = msg->ops_AttrList;

    data = (struct DrawerIconClassData *) INST_DATA(cl, obj);

    while ((tag = NextTagItem(&tstate)))
    {
        switch (tag->ti_Tag)
        {
            default:
                retval = DoSuperMethodA(cl, obj, (Msg) msg);
                break;
        }
    }

    return retval;
}

IPTR drawerIconGet(Class * cl, Object * obj, struct opGet * msg)
{
    IPTR            retval = 1;
    struct DrawerIconClassData *data;

    data = (struct DrawerIconClassData *) INST_DATA(cl, obj);

    switch (msg->opg_AttrID)
    {
        default:
            retval = DoSuperMethodA(cl, obj, (Msg) msg);
            break;
    }

    return retval;
}

IPTR drawerIconDispose(Class * cl, Object * obj, Msg msg)
{
    IPTR            retval;

    retval = DoSuperMethodA(cl, obj, msg);

    return retval;
}

BOOPSI_DISPATCHER(IPTR, drawerIconDispatcher, cl, obj, msg)
{
    ULONG           retval = 0;

    switch (msg->MethodID)
    {
        case OM_NEW:
            retval = drawerIconNew(cl, obj, (struct opSet *) msg);
            break;
        case OM_SET:
            retval = drawerIconSet(cl, obj, (struct opSet *) msg);
            break;
        case OM_GET:
            retval = drawerIconGet(cl, obj, (struct opGet *) msg);
            break;
        case OM_DISPOSE:
            retval = drawerIconDispose(cl, obj, msg);
            break;
        default:
            retval = DoSuperMethodA(cl, obj, msg);
            break;
    }

    return retval;
}
