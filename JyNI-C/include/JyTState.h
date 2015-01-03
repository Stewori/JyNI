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


/*
 * JyTState.h
 *
 *  Created on: 02.09.2014
 *      Author: Stefan Richthofer
 */

#ifndef JYTSTATE_H_
#define JYTSTATE_H_

#define TS_GET_JY(ts) ((jobject) (ts)->frame)
#define TS_SET_JY(ts, jy) ((ts)->frame = (struct _frame*) (jy))

//struct _ts *next;
//PyInterpreterState *interp;
//struct _frame *frame;
//int recursion_depth;
//int tracing;
//int use_tracing;
//Py_tracefunc c_profilefunc;
//Py_tracefunc c_tracefunc;
//PyObject *c_profileobj;
//PyObject *c_traceobj;
//PyObject *curexc_type;
//PyObject *curexc_value;
//PyObject *curexc_traceback;
#define TS_TRUNCATED_SIZE (sizeof(struct _ts*) + sizeof(PyInterpreterState*) \
		+ sizeof(struct _frame*) + 3*sizeof(int) + 2*sizeof(Py_tracefunc) \
		+ 5*sizeof(PyObject*))

inline void JyErr_InsertCurExc();
inline void Py_SetRecursionLimitNative(int new_limit);

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeRecursionLimit
 * Signature: (I)V
 */
void JyTState_setNativeRecursionLimit(JNIEnv *env, jclass class, jint recursionLimit);

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeCallDepth
 * Signature: (JI)V
 */
void JyTState_setNativeCallDepth(JNIEnv *env, jclass class, jlong nativeThreadState, jint callDepth);

/*
 * Class:     JyNI_JyNI
 * Method:    initNativeThreadState
 * Signature: (LJyNI/JyTState;Lorg/python/core/ThreadState;)J
 */
jlong JyTState_initNativeThreadState(JNIEnv *env, jclass class, jobject jyTState, jobject threadState);

/*
 * Class:     JyNI_JyNI
 * Method:    clearNativeThreadState
 * Signature: (J)V
 */
void JyTState_clearNativeThreadState(JNIEnv *env, jclass class, jlong threadState);

#endif /* JYTSTATE_H_ */
