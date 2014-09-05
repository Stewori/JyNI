/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014 Stefan Richthofer.  All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014 Python Software Foundation.  All rights reserved.
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
 * JyTState.c
 *
 *  Created on: 02.09.2014
 *      Author: Stefan Richthofer
 */

#include <JyNI.h>

inline void JyErr_InsertCurExc()
{
	PyThreadState *tstate = PyThreadState_GET();
	env();
	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNIJyErr_InsertCurExc,
			TS_GET_JY(tstate),
			JyNI_JythonPyObject_FromPyObject(tstate->curexc_type),
			JyNI_JythonPyObject_FromPyObject(tstate->curexc_value),
			JyNI_JythonPyObject_FromPyObject(tstate->curexc_traceback));
}

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeRecursionLimit
 * Signature: (I)V
 */
void JyTState_setNativeRecursionLimit(JNIEnv *env, jclass class, jint recursionLimit)
{
	Py_SetRecursionLimitNative(recursionLimit);
}

/*
 * Class:     JyNI_JyNI
 * Method:    setNativeCallDepth
 * Signature: (JI)V
 */
void JyTState_setNativeCallDepth(JNIEnv *env, jclass class, jlong nativeThreadState, jint callDepth)
{
	((PyThreadState*) nativeThreadState)->recursion_depth = callDepth;
}

/*
 * Class:     JyNI_JyNI
 * Method:    initNativeThreadState
 * Signature: (LJyNI/JyTState;Lorg/python/core/ThreadState;)J
 */
jlong JyTState_initNativeThreadState(JNIEnv *env, jclass class, jobject jyTState, jobject threadState)
{
	//TODO jyTState is currently not used here. Maybe clean it away later...
//	jputs("init native ThreadState...");
	PyThreadState* tstate = (PyThreadState*) PyObject_RawMalloc(TS_TRUNCATED_SIZE);
	tstate->next = NULL;
	tstate->interp = NULL;
	tstate->frame = NULL;
	tstate->recursion_depth = 0;
	//int tracing = 0;
	//int use_tracing = 0;
	//Py_tracefunc c_profilefunc = NULL;
	//Py_tracefunc c_tracefunc = NULL;
	tstate->c_profileobj = NULL;
	tstate->c_traceobj = NULL;
	tstate->curexc_type = NULL;
	tstate->curexc_value = NULL;
	tstate->curexc_traceback = NULL;
	TS_SET_JY(tstate, (*env)->NewWeakGlobalRef(env, threadState));
//	jputs("done");
//	jputsLong(tstate);
	return (jlong) tstate;
}

/*
 * Class:     JyNI_JyNI
 * Method:    clearNativeThreadState
 * Signature: (J)V
 */
void JyTState_clearNativeThreadState(JNIEnv *env, jclass class, jlong threadState)
{
	(*env)->DeleteWeakGlobalRef(env, TS_GET_JY((PyThreadState*) threadState));
	PyObject_RawFree((void*) threadState);
}
