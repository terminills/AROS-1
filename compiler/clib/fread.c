/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: ANSI C function fread()
    Lang: english
*/

#include <errno.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "__errno.h"
#include "__stdio.h"
#include "__open.h"

/*****************************************************************************

    NAME */
#include <stdio.h>

	size_t fread (

/*  SYNOPSIS */
	void * buf,
	size_t size,
	size_t nblocks,
	FILE * stream)

/*  FUNCTION
	Read an amount of bytes from a stream.

    INPUTS
	buf - The buffer to read the bytes into
	size - Size of one block to read
	nblocks - The number of blocks to read
	stream - Read from this stream

    RESULT
	The number of blocks read. This may range from 0 when the stream
	contains no more blocks up to nblocks. In case of an error, 0 is
	returned.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	fopen(), fwrite()

    INTERNALS

    HISTORY
	15.12.1996 digulla created

******************************************************************************/
{
    GETUSER;

    size_t cnt;
    fdesc *fdesc = __getfdesc(stream->fd);

    if (!fdesc)
    {
	stream->flags |= _STDIO_ERROR;
	errno = EBADF;
	return 0;
    }

    cnt = FRead ((BPTR)fdesc->fh, buf, size, nblocks);

    if (cnt == -1)
    {
	errno = IoErr2errno (IoErr ());
	stream->flags |= _STDIO_ERROR;

	cnt = 0;
    }
    else if (cnt == 0)
    {
	stream->flags |= _STDIO_EOF;
    }

    return cnt;
} /* fread */

