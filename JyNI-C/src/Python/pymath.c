/* This File is based on pymath.c from CPython 2.7.8 release.
 * It has been modified to suit JyNI needs.
 *
 * Copyright of the original file:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */


#include "JyNI.h"

#ifdef X87_DOUBLE_ROUNDING
/* On x86 platforms using an x87 FPU, this function is called from the
   Py_FORCE_DOUBLE macro (defined in pymath.h) to force a floating-point
   number out of an 80-bit x87 FPU register and into a 64-bit memory location,
   thus rounding from extended precision to double precision. */
double _Py_force_double(double x)
{
    volatile double y;
    y = x;
    return y;
}
#endif

#ifdef HAVE_GCC_ASM_FOR_X87

/* inline assembly for getting and setting the 387 FPU control word on
   gcc/x86 */

unsigned short _Py_get_387controlword(void) {
    unsigned short cw;
    __asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
    return cw;
}

void _Py_set_387controlword(unsigned short cw) {
    __asm__ __volatile__ ("fldcw %0" : : "m" (cw));
}

#endif


#ifndef HAVE_HYPOT
double hypot(double x, double y)
{
    double yx;

    x = fabs(x);
    y = fabs(y);
    if (x < y) {
        double temp = x;
        x = y;
        y = temp;
    }
    if (x == 0.)
        return 0.;
    else {
        yx = y/x;
        return x*sqrt(1.+yx*yx);
    }
}
#endif /* HAVE_HYPOT */

#ifndef HAVE_COPYSIGN
double
copysign(double x, double y)
{
    /* use atan2 to distinguish -0. from 0. */
    if (y > 0. || (y == 0. && atan2(y, -1.) > 0.)) {
        return fabs(x);
    } else {
        return -fabs(x);
    }
}
#endif /* HAVE_COPYSIGN */

#ifndef HAVE_ROUND
double
round(double x)
{
    double absx, y;
    absx = fabs(x);
    y = floor(absx);
    if (absx - y >= 0.5)
    y += 1.0;
    return copysign(y, x);
}
#endif /* HAVE_ROUND */
