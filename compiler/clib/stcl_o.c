/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: SAS C function stcl_o()
    Lang: english
*/

#include <stdio.h>

/*****************************************************************************

    NAME */
#include <string.h>

	int stcl_o (

/*  SYNOPSIS */
	char 		* out,
	long	    lvalue)

/*  FUNCTION
	Convert an long integer to an octal string

    INPUTS
	out     - Result will be put into this string
	uivalue - the value to convert

    RESULT
	The number of characters written into the string

    NOTES
	SAS C specific
	
    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	15.12.2000 stegerg created.

******************************************************************************/
{
 
    return sprintf(out, "%lo", lvalue);
    
} /* stcl_o */
