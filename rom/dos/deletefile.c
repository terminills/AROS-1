/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Delete a file or directory.
    Lang: English
*/
#include <exec/memory.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <dos/dos.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(BOOL, DeleteFile,

/*  SYNOPSIS */
	AROS_LHA(CONST_STRPTR, name, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 12, Dos)

/*  FUNCTION
	Tries to delete a file or directory by a given name.
	May fail if the file is in use or protected from deletion.

    INPUTS
	name	   - NUL terminated name.

    RESULT
	!= 0 if the file is gone, 0 if is still there.
	IoErr() gives additional information in that case.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    /* Get pointer to I/O request. Use stackspace for now. */
    struct IOFileSys iofs;

    /* Prepare I/O request. */
    InitIOFS(&iofs, FSA_DELETE_OBJECT, DOSBase);

    if(DoName(&iofs, name, DOSBase) == 0)
	return DOSTRUE;
    else
	return DOSFALSE;

    AROS_LIBFUNC_EXIT
} /* DeleteFile */
