/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Check if a double is infinite.
    Lang: english
*/

/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

/*
 * isinf(x) returns 1 is x is inf, else 0;
 * no branching!
 */

#include "__math.h"
#include <math.h>

int __isinf(double val)
{
    int hx,lx;

    hx = (__HI(val)&0x7fffffff);
    lx = __LO(val);
    hx &= 0x7fffffff;
    hx ^= 0x7ff00000;
    hx |= lx;

    return (hx == 0);
} /* __isinf */
