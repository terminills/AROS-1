/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: DOS function InternalLoadSeg()
    Lang: english
*/


#define DEBUG 0

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <aros/debug.h>
#include "dos_intern.h"
#include "internalloadseg.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

        AROS_LH4(BPTR, InternalLoadSeg,

/*  SYNOPSIS */
        AROS_LHA(BPTR       , fh           , D0),
        AROS_LHA(BPTR       , table        , A0),
        AROS_LHA(LONG_FUNC *, funcarray    , A1),
        AROS_LHA(LONG *     , stack        , A2),

/*  LOCATION */
        struct DosLibrary *, DOSBase, 126, Dos)

/*  FUNCTION
        Loads from fh.
        Functionarray is a pointer to an array of functions. See below.

        This function really only tries to load the different file
        formats aos, elf and aout.

    INPUTS
        fh            : Filehandle to load from
        table         : ignored
        funcarray : array of functions to be used for read, seek, alloc and free
           FuncTable[0] -> bytes  = ReadFunc(readhandle, buffer, length), DOSBase
                           D0                D1          A0      D0       A6
           FuncTable[1] -> Memory = AllocFunc(size,flags), ExecBase
                           D0                 D0   D1      A6
           FuncTable[2] -> FreeFunc(memory, size), ExecBase
                                    A1       D0    A6
           FuncTable[3] -> pos    = SeekFunc(readhandle, pos, mode), DOSBase
                           D0                D0          D1   D2
        stack         : pointer to storage (ULONG) for stacksize.
                        (currently ignored)

    RESULT
        seglist  - pointer to loaded Seglist or NULL in case of failure.

    NOTES
        FuncTable[3] is not used for Amiga HUNK format files, but is required
                     for ELF.

    EXAMPLE

    BUGS
       Use of table and stack are not implemented, yet!

    SEE ALSO
        UnLoadSeg()

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    typedef struct _segfunc_t
    {
    	ULONG id;
        BPTR (*func)(BPTR, BPTR, SIPTR *, SIPTR *, struct DosLibrary *);
        D(CONST_STRPTR format;)
    } segfunc_t;

    #define SEGFUNC(id, format) {id, InternalLoadSeg_##format D(, (STRPTR)#format)}
    
    static const segfunc_t funcs[] = 
    {
        SEGFUNC(0x7f454c46, ELF),
        SEGFUNC(0x000003f3, AOS)
    };
  
    BPTR segs = 0;

    if (fh)
    {
        UBYTE i;
	const UBYTE num_funcs = sizeof(funcs) / sizeof(funcs[0]);
    	ULONG id;
	LONG len;

	SetIoErr(0);
    	len = ilsRead(fh, &id, sizeof(id));
	if (len == sizeof(id)) {
	    id = AROS_BE2LONG(id);
	    for (i = 0; i < num_funcs; i++) {
		if (funcs[i].id == id) {
		    segs = (*funcs[i].func)(fh, BNULL, (SIPTR *)funcarray,
			stack, DOSBase);
		    D(bug("[InternalLoadSeg] %s loading %p as an %s object.\n",
			segs ? "Succeeded" : "FAILED", fh, funcs[i].format));
		    return segs;
 		}
 	    }
 	}
    }

    SetIoErr(ERROR_NOT_EXECUTABLE);
    return BNULL;
  
    AROS_LIBFUNC_EXIT
} /* InternalLoadSeg */

APTR _ilsAllocVec(SIPTR *funcarray, ULONG size, ULONG req)
{
    UBYTE *p = ilsAllocMem(size, req);
    if (!p)
    	return NULL;
    D(bug("allocmem %p %d\n", p, size));
    *((ULONG*)p) = (ULONG)size;
    return p + sizeof(ULONG);       
}

void _ilsFreeVec(SIPTR *funcarray, void *buf)
{
    UBYTE *p = (UBYTE*)buf;
    ULONG size;
    if (!buf)
    	return;
    p -= sizeof(ULONG);
    size = ((ULONG*)p)[0];
    D(bug("freemem %p %d\n", p, size));
    if (!size)
    	return;

    ilsFreeMem(p, size);
}
