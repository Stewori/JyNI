/* This File is based on timefuncs.h from CPython 2.7.13 release.
 * It has been modified to suit JyNI needs.
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

/*  timefuncs.h
 */

/* Utility function related to timemodule.c. */

#ifndef TIMEFUNCS_H
#define TIMEFUNCS_H
#ifdef __cplusplus
extern "C" {
#endif

/* Cast double x to time_t, but raise ValueError if x is too large
 * to fit in a time_t.  ValueError is set on return iff the return
 * value is (time_t)-1 and PyErr_Occurred().
 */
PyAPI_FUNC(time_t) _PyTime_DoubleToTimet(double x);

/* Get the current time since the epoch in seconds */
//PyAPI_FUNC(double) _PyTime_FloatTime(void);


#ifdef __cplusplus
}
#endif
#endif  /* TIMEFUNCS_H */
