/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: ANSI C function strcpy()
    Lang: english
*/

/*****************************************************************************

    NAME */
#include <string.h>

	char * strcpy (

/*  SYNOPSIS */
	char	   * dest,
	const char * src)

/*  FUNCTION
	Copy a string. Works like an assignment "dest=src;".

    INPUTS
	dest - The string is copied into this variable. Make sure it is
		large enough.
	src - This is the new contents of dest.

    RESULT
	dest.

    NOTES
	No check is beeing made that dest is large enough for src.

    EXAMPLE

    BUGS

    SEE ALSO
	strncpy(), memcpy(), memmove()

    INTERNALS

    HISTORY
	29.07.1996 digulla created

******************************************************************************/
{
    char * ptr = dest;

    while ((*ptr = *src))
    {
	ptr ++;
	src ++;
    }

    return dest;
} /* strcpy */
