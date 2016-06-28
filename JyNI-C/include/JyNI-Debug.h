/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
 * 2010, 2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
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


/*
 * JyNI-Debug.h
 *
 *  Created on: 21.11.2015
 *      Author: Stefan Richthofer
 */

#ifndef JYNI_DEBUG_H_
#define JYNI_DEBUG_H_

#include <execinfo.h>
/* We define some debug macros for JyNI: */
char mbuf[100];

#define XSTR(s) STR(s)
#define STR(s) #s
#define debug_log(message) jputs(message)
//jputs(XSTR(__LINE__) message)
//#define call_log(fkt) (debug_log(__LINE__ ## :  ## fkt), fkt)
#define call_log(fkt) (debug_log((sprintf(mbuf, "%d: %s", __LINE__, #fkt), mbuf)), fkt)
//(debug_log(#fkt), fkt)
#define fkt_log debug_log((sprintf(mbuf, "\n%s (%d)", __FUNCTION__, __LINE__), mbuf));
#define msg_log(msg) debug_log((sprintf(mbuf, "%d (%s): %s", __LINE__, __FUNCTION__, msg), mbuf));
#define debugPy(obj) debug_log((sprintf(mbuf, "%s %lld (%s)", __FUNCTION__, (jlong) obj, Py_TYPE(obj)->tp_name), mbuf));
#define debugContext(pre, obj, post) debug_log((sprintf(mbuf, "%s %lld (%s) %s", pre, (jlong) obj, Py_TYPE(obj)->tp_name, post), mbuf));


//#define jputs(msg) (_jputs(__FUNCTION__), _jputs(msg))
#define jputs(msg) _jputs(msg)
inline void _jputs(const char* msg);
//#define jputsLong(val) \
//	jputs(__FUNCTION__); \
//	jputsLong0(val)
inline void jputsLong(jlong val);
inline void jputsPy(PyObject* o);
inline void putsPy(PyObject* o);

void jPrintCStackTrace();

#endif /* JYNI_DEBUG_H_ */
