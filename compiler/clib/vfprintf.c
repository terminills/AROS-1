/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Format a string and call a usercallback to output each char
    Lang: english
*/
/* Original source from libnix */

#define AROS_ALMOST_COMPATIBLE

#include <proto/dos.h>
#include <errno.h>
#include "__errno.h"
#include "__open.h"

static int __putc(int c, BPTR fh);

/*****************************************************************************

    NAME */
#include <stdio.h>
#include <stdarg.h>

	int vfprintf (

/*  SYNOPSIS */
	FILE	   * stream,
	const char * format,
	va_list      args)

/*  FUNCTION
	Format a list of arguments and print them on the specified stream.

    INPUTS
	stream - A stream on which one can write
	format - A printf() format string.
	args - A list of arguments for the format string.

    RESULT
	The number of characters written.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	06.12.1996 digulla copied from libnix

******************************************************************************/
{
    fdesc *fdesc = __getfdesc(stream->fd);

    if (!fdesc)
    {
        GETUSER;

	errno = EBADF;
	return 0;
    }

    return __vcformat (fdesc->fh, __putc, format, args);
} /* vfprintf */


static int __putc(int c, BPTR fh)
{
    if (FPutC(fh, c) == EOF)
    {
    	GETUSER;

	errno = IoErr2errno(IoErr());
	return EOF;
    }

    return c;
}
