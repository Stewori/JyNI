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
 * JyTState.c
 *
 *  Created on: 02.09.2014
 *      Author: Stefan Richthofer
 */

#include <JyNI.h>

inline void JyErr_InsertCurExc()
{
//	jputs(__FUNCTION__);
	PyThreadState *tstate = PyThreadState_GET();
	PyObject *curexc_type, *curexc_value, *curexc_traceback;
	PyErr_Fetch(&curexc_type, &curexc_value, &curexc_traceback);
	env();
	(*env)->CallStaticVoidMethod(env, JyNIClass, JyNIJyErr_InsertCurExc,
			TS_GET_JY(tstate),
			JyNI_JythonPyObject_FromPyObject(curexc_type),
			JyNI_JythonPyObject_FromPyObject(curexc_value),
			JyNI_JythonPyObject_FromPyObject(curexc_traceback));
	Py_XDECREF(curexc_type);
	Py_XDECREF(curexc_value);
	Py_XDECREF(curexc_traceback);
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
	//PyThreadState* tstate = (PyThreadState*) PyObject_RawMalloc(TS_TRUNCATED_SIZE);
	PyThreadState* tstate = (PyThreadState*) malloc(TS_TRUNCATED_SIZE);
	tstate->next = NULL;
	tstate->interp = NULL;
	tstate->frame = NULL;
	tstate->recursion_depth = 0;
	//int tracing = 0;
	/* We directly init this to 1, which is the Java-side keep-alive anchor.
	 * It will be released in JyTState_clearNativeThreadState.
	 */
	tstate->JyNI_gilstate_counter = 1;
	//int use_tracing = 0;
	tstate->JyNI_natively_attached = 0;
	//Py_tracefunc c_profilefunc = NULL;
	//Py_tracefunc c_tracefunc = NULL;
	tstate->c_profileobj = NULL;
	tstate->c_traceobj = NULL;
	tstate->curexc_type = NULL;
	tstate->curexc_value = NULL;
	tstate->curexc_traceback = NULL;
	tstate->exc_type = NULL; // (not used)
	tstate->exc_value = NULL; // (not used)
	tstate->exc_traceback = NULL; // (not used)
	tstate->dict = NULL;
	TS_SET_JY(tstate, (*env)->NewWeakGlobalRef(env, threadState));
//	jputs("done");
//	jputsLong(tstate);
	return (jlong) tstate;
}

/*
 * Class:     JyNI_JyNI
 * Method:    clearNativeThreadState
 * Signature: (J)V
 *
 * Don't call this method from native code!
 * It decrements the gilstate counter to release the Java-side keep-alive anchor
 * of the thread state. Thus it must only be called from Java.
 */
void JyTState_clearNativeThreadState(JNIEnv *env, jclass class, jlong threadState)
{
	(*env)->DeleteWeakGlobalRef(env, TS_GET_JY((PyThreadState*) threadState));
	if (!--((PyThreadState*) threadState)->JyNI_gilstate_counter)
	{
		PyThreadState_Clear((PyThreadState*) threadState);
		_delNativeThreadState((PyThreadState*) threadState);
	}
}

void _delNativeThreadState(PyThreadState* threadState)
{
	//PyObject_RawFree(threadState);
	free(threadState);
}
