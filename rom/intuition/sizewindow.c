/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include "intuition_intern.h"

/*****************************************************************************

    NAME */
#include <proto/intuition.h>

	AROS_LH3(void, SizeWindow,

/*  SYNOPSIS */
	AROS_LHA(struct Window *, window, A0),
	AROS_LHA(LONG           , dx, D0),
	AROS_LHA(LONG           , dy, D1),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 48, Intuition)

/*  FUNCTION
	Modify the size of a window by the specified offsets.

    INPUTS
	window - The window to resize.
	dx - Add this to the width.
	dy - Add this to the height.

    RESULT
	None.

    NOTES
	The resize of the window may be delayed. If you depend on the
	information that is has changed size, wait for IDCMP_NEWSIZE.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

    struct IntuiActionMessage * msg;
 
    if (dx || dy)
    {   
	msg = AllocIntuiActionMsg(AMCODE_SIZEWINDOW, window, IntuitionBase);

	if (NULL != msg)
	{
	    msg->iam_SizeWindow.dx = dx;
	    msg->iam_SizeWindow.dy = dy;

	    SendIntuiActionMsg(msg, IntuitionBase); 
	}   
    }

    AROS_LIBFUNC_EXIT
    
} /* SizeWindow */
