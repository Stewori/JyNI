/* This File is based on pymath.c from CPython 2.7 (latest repository version).
 * It has been modified to suit JyNI needs.
 * Note that this file was generated by JyNIgate and its source is hosted in the
 * JyNIgate project. Modifying this generated file is not recommended.
 *
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
 * All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
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
