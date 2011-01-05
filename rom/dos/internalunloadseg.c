/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/

#include <aros/libcall.h>
#include <aros/asmcall.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/kernel.h>

#include "dos_intern.h"
#include "internalloadseg.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

        AROS_LH2(BOOL, InternalUnLoadSeg,

/*  SYNOPSIS */
        AROS_LHA(BPTR     , seglist , D1),
        AROS_LHA(VOID_FUNC, freefunc, A1),

/*  LOCATION */
        struct DosLibrary *, DOSBase, 127, Dos)

/*  FUNCTION
	Unloads a seglist loaded with InternalLoadSeg().

    INPUTS
	seglist  - Seglist
	freefunc - Function to be called to free memory

    RESULT
	DOSTRUE if everything wents O.K.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    BPTR next;

    if (seglist)
    {
#ifdef KrnUnregisterModule
	if (KernelBase)
	    KrnUnregisterModule(seglist);
#endif

#if (AROS_FLAVOUR & AROS_FLAVOUR_BINCOMPAT)
	{
    	    /* free overlay structures */
    	    ULONG *seg = BADDR(seglist);
    	    if (seg[2] == 0x0000abcd && seg[6] == (ULONG)DOSBase->dl_GV) {
    	    	Close((BPTR)seg[3]); /* file handle */
    	    	loadseg_free((SIPTR*)freefunc, (void*)seg[4]); /* overlay table, APTR */
    	    	loadseg_free((SIPTR*)freefunc, BADDR(seg[5])); /* hunk table, BPTR */
    	    }
    	}
#endif

	while (seglist)
	{
	    next = *(BPTR *)BADDR(seglist);
	    loadseg_free((SIPTR*)freefunc, BADDR(seglist));
	    seglist = next;
	}

	return DOSTRUE;
    }
    else
	return DOSFALSE;

    AROS_LIBFUNC_EXIT
} /* InternalUnLoadSeg */
