/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: ANSI C function puts()
    Lang: english
*/

/*****************************************************************************

    NAME */
#include <stdio.h>

	int puts (

/*  SYNOPSIS */
	const char * str,
	FILE * fh)

/*  FUNCTION
	Print a string to fh. A newline ('\n') is emmitted after the string.

    INPUTS
	str - Print this string
	fh - Write to this stream

    RESULT
	> 0 on success and EOF on error. On error, the reason is put in
	errno.

    NOTES

    EXAMPLE
	#include <errno.h>

	if (puts ("Hello World.") != EOF)
	    fprintf (stderr, "Success");
	else
	    fprintf (stderr, "Failure: errno=%d", errno);

    BUGS

    SEE ALSO
	fputs(), printf(), fprintf(), putc(), fputc()

    INTERNALS

    HISTORY
	10.12.1996 digulla created after libnix

******************************************************************************/
{
    if (fputs (str, fh) == EOF)
	return EOF;

    if (putc ('\n', fh) == EOF)
	return EOF;

    if (fflush (stdout) == EOF)
	return EOF;

    return 1;
} /* puts */

