/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/

#include <intuition/classusr.h>

#include "muimaster_intern.h"

/*****************************************************************************

    NAME */
#ifndef _AROS
__asm Object *MUI_MakeObjectA(register __d0 LONG type, register __a0 ULONG *params)
#else
	AROS_LH2(Object *, MUI_MakeObjectA,

/*  SYNOPSIS */
	AROS_LHA(LONG, type, D0),
	AROS_LHA(ULONG *, params, A0),

/*  LOCATION */
	struct MUIMasterBase *, MUIMasterBase, 20, MUIMaster)
#endif

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS
	The function itself is a bug ;-) Remove it!

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct MUIMasterBase *,MUIMasterBase)

#if 0
    switch (type)
    {
	case MUIO_Label: /* STRPTR label, ULONG flags */
	{
	    struct TagItem tags[5];
	    int ntags = 0;

	    if (params[1] & MUIO_Label_SingleFrame)
	    {
		tags[ntags].ti_Tag = MUIA_Frame;
		tags[ntags].ti_Data = (IPTR)MUIV_Frame_ImageButton;
		ntags++;
	    }
	    else if (params[1] & MUIO_Label_DoubleFrame)
	    {
		tags[ntags].ti_Tag = MUIA_Frame;
		tags[ntags].ti_Data = (IPTR)MUIV_Frame_String;
		ntags++;
	    }

	    if (params[1] & MUIO_Label_LeftAligned)
	    {
		tags[ntags].ti_Tag = MUIA_Text_PreParse;
		tags[ntags].ti_Data = (IPTR)"\33l";
		ntags++;
	    }
	    else if (params[1] & MUIO_Label_Centered)
	    {
		tags[ntags].ti_Tag = MUIA_Text_PreParse;
		tags[ntags].ti_Data = (IPTR)"\33c";
		ntags++;
	    }
	    else
	    {
		tags[ntags].ti_Tag = MUIA_Text_PreParse;
		tags[ntags].ti_Data = (IPTR)"\33r";
		ntags++;
	    }

	    if (params[1] & MUIO_Label_FreeVert)
	    {
		tags[ntags].ti_Tag = MUIA_Text_SetVMax;
		tags[ntags].ti_Data = (IPTR)FALSE;
		ntags++;
	    }

	    tags[ntags].ti_Tag = TAG_DONE;
	    tags[ntags].ti_Data = 0;

	    return MUI_NewObject(MUIC_Text,
		MUIA_FramePhantomHoriz, TRUE,
		MUIA_Text_HiChar, params[1] & 0xff,
		MUIA_Text_Contents, params[0],
		TAG_MORE, tags,
		TAG_DONE);
	    break;
	}

	case MUIO_Button: /* STRPTR label */
	{
	    return MUI_NewObject(MUIC_Text,
		ButtonFrame,
		MUIA_Font, MUIV_Font_Button,
		MUIA_Text_HiCharIdx, '_',
		MUIA_Text_Contents, params[0],
		MUIA_Text_PreParse, "\33c",
		MUIA_InputMode    , MUIV_InputMode_RelVerify,
		MUIA_Background   , MUII_ButtonBack,
		TAG_DONE);
	}

	case MUIO_Checkmark: /* STRPTR label */
	{
	    return MUI_NewObject(MUIC_Group,
		MUIA_Group_Horiz,TRUE,
		Child, Label(params[0]),
		Child, MUI_NewObject(MUIC_Image,
	            MUIA_Image_Spec, MUII_CheckMark,
	            MUIA_InputMode, MUIV_InputMode_Toggle,
	            MUIA_Frame, MUIV_Frame_None,
	            MUIA_ShowSelState, FALSE,
		    TAG_DONE),
		TAG_DONE);
	    break;
	}

	case MUIO_Cycle: /* STRPTR label, STRPTR *entries */
	    return MUI_NewObject(MUIC_Group,
		MUIA_Group_Horiz,TRUE,
		Child, LLabel1(params[0]),
		Child, MUI_NewObject(MUIC_Cycle,
		    MUIA_Cycle_Entries, params[1],
		    TAG_DONE),
		TAG_DONE);
	    break;

	case MUIO_Radio: /* STRPTR label, STRPTR *entries */
	    break;

	case MUIO_Slider: /* STRPTR label, LONG min, LONG max */
	    break;

	case MUIO_String: /* STRPTR label, LONG maxlen */
	    break;

	case MUIO_PopButton: /* STRPTR imagespec */
	    return MUI_NewObject(MUIC_Image,
		MUIA_Image_Spec, params[0],
		MUIA_InputMode, MUIV_InputMode_RelVerify,
		MUIA_ShowSelState, FALSE,
		TAG_DONE);
	    break;

	case MUIO_HSpace: /* LONG space   */
	    return MUI_NewObject(MUIC_Rectangle,
		MUIA_FixWidth, params[0],
		TAG_DONE);
	    break;

	case MUIO_VSpace: /* LONG space   */
	    return MUI_NewObject(MUIC_Rectangle,
		MUIA_FixHeight, params[0],
		TAG_DONE);
	    break;

	case MUIO_HBar: /* LONG space   */
	    return MUI_NewObject(MUIC_Rectangle,
		MUIA_Rectangle_HBar, TRUE,
		MUIA_FixHeight, params[0],
		TAG_DONE);
	    break;

	case MUIO_VBar: /* LONG space   */
	    return MUI_NewObject(MUIC_Rectangle,
		MUIA_Rectangle_VBar, TRUE,
		MUIA_FixWidth, params[0],
		TAG_DONE);
	    break;

	case MUIO_MenustripNM: /* struct NewMenu *nm, ULONG flags */
	    break;

	case MUIO_Menuitem: /* STRPTR label, STRPTR shortcut, ULONG flags, ULONG data  */
	    break;

	case MUIO_BarTitle: /* STRPTR label */
	    return MUI_NewObject(MUIC_Rectangle,
		MUIA_Rectangle_HBar, TRUE,
		MUIA_Rectangle_BarTitle, params[0],
		TAG_DONE);
	    break;

	case MUIO_NumericButton: /* STRPTR label, LONG min, LONG max, STRPTR format */
	    break;

	default:
#warning FIXME: complain loud for missing/wrong MUIO_something
    }
#endif

    return NULL;

    AROS_LIBFUNC_EXIT

} /* MUIA_MakeObjectA */
