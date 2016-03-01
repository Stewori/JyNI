/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
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
 * JyRefMonitor.h
 *
 *  Created on: 20.05.2014
 *      Author: Stefan Richthofer
 */
#include <JyNI.h>

#define JY_INC_MASK        1
#define JY_DEC_MASK        2
#define JY_MEMORY_MASK     4
#define JY_NATIVE_MASK     8
#define JY_GC_MASK        16
#define JY_PRE_MASK       32
#define JY_POST_MASK      64
#define JY_FINALIZE_MASK 128
#define JY_INLINE_MASK   256
#define JY_IMMORTAL_MASK 512

#define JY_NATIVE_ALLOC      13  // INC_MASK | MEMORY_MASK | NATIVE_MASK
#define JY_NATIVE_FREE       14  // DEC_MASK | MEMORY_MASK | NATIVE_MASK
#define JY_NATIVE_REALLOC    15  // JY_NATIVE_ALLOC | JY_NATIVE_FREE
#define JY_NATIVE_ALLOC_GC   29  // JY_NATIVE_ALLOC | GC_MASK
#define JY_NATIVE_FREE_GC    30  // JY_NATIVE_FREE | GC_MASK
#define JY_NATIVE_REALLOC_GC 31  // JY_NATIVE_REALLOC | GC_MASK
#define JY_NATIVE_FINALIZE  168  // NATIVE_MASK | FINALIZE_MASK | PRE_MASK
#define JY_NATIVE_CLEAR     184  // FINALIZE_MASK | GC_MASK
#define JY_NATIVE_INCREF      9  // INC_MASK | NATIVE_MASK
#define JY_NATIVE_DECREF     10  // DEC_MASK | NATIVE_MASK
#define JY_NATIVE_GC_TRACK   25  // JY_GC_MASK | INC_MASK | NATIVE_MASK
#define JY_NATIVE_GC_UNTRACK 26  // JY_GC_MASK | DEC_MASK | NATIVE_MASK

#define IsJyNIDebug(flags) Jy_memDebugFlags

#define JyNIDebug(flags, pref, ref, size, tpName) \
	if (Jy_memDebugFlags) JyRefMonitor_addAction(flags, pref, ref, size, tpName, __FUNCTION__, __FILE__, __LINE__)

#define JyNIDebug2(flags, ref1, ref2, size, tpName) \
	if (Jy_memDebugFlags) JyRefMonitor_addAction2(flags, ref1, ref2, size, tpName, __FUNCTION__, __FILE__, __LINE__)

#define JyNIDebugOp(flags, op, size) \
	JyNIDebug(flags, op, AS_JY(op), size, Py_TYPE(op) ? Py_TYPE(op)->tp_name : NULL)

#define JyNIDebugOp2(flags, op, ref2, size) \
	JyNIDebug2(flags, AS_JY(op), ref2, size, Py_TYPE(op) ? Py_TYPE(op)->tp_name : NULL)

extern jint Jy_memDebugFlags;

// Call-in
void JyRefMonitor_setMemDebug(JNIEnv *env, jclass class, jint flags);

// Memory-tracking
void JyRefMonitor_addAction(jshort action, PyObject* op, JyObject* object,
		size_t size, char* type, char* function, char* file, jint line);
void JyRefMonitor_addAction2(jshort action, JyObject* object, JyObject* object2,
		size_t size, char* type, char* function, char* file, jint line);
