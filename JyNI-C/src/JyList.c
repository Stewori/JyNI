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
 * JyList.c
 *
 *  Created on: 16.05.2013, 17:51:35
 *      Author: Stefan Richthofer
 */

#include "JyNI.h"

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_get
 * Signature: (JI)Lorg/python/core/PyObject;
 */
jobject JyList_get(JNIEnv *env, jclass class, jlong handle, jint index)
{
//	jputs(__FUNCTION__);
//	jputsLong(((PyObject*) handle)->ob_refcnt);
//	jputsLong(handle);
//	jputsLong(index);
	// PyList_GET_ITEM returns a borrowed reference, so no need to decref it here.
	//PyObject* result = PyList_GET_ITEM((PyObject*) handle, index);
	//if (!result) jputs("result is NULL");
	//jputsLong(PyList_GET_ITEM((PyObject*) handle, 0));
//	jputs(Py_TYPE(result)->tp_name);
	//jobject er = JyNI_JythonPyObject_FromPyObject(result);
//	if ((*env)->ExceptionCheck(env)) {
//		jputs("Exception happened....");
//		(*env)->ExceptionDescribe(env);
//	}
//	if ((*env)->IsSameObject(env, er, NULL)) jputs("er is NULL");
	return JyNI_JythonPyObject_FromPyObject(PyList_GET_ITEM((PyObject*) handle, index));
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_size
 * Signature: (J)I
 */
jint JyList_size(JNIEnv *env, jclass class, jlong handle)
{
	return (jint) PyList_GET_SIZE((PyObject*) handle);
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_set
 * Signature: (JILorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
jobject JyList_set(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
//	jputs(__FUNCTION__);
//	jputsLong(handle);
//	jputsLong(index);
	jobject old = JyNI_JythonPyObject_FromPyObject(PyList_GET_ITEM((PyObject*) handle, index));
	Py_XDECREF(PyList_GET_ITEM((PyObject*) handle, index));
	PyObject* op = (PyObject*) pyObj;
	if (op) Py_INCREF(op);
	else op = JyNI_PyObject_FromJythonPyObject(obj);
	//PyList_SetItem((PyObject*) handle, index, op);
	PyList_SET_ITEM((PyObject*) handle, index, op);
	updateJyGCHeadLink((PyObject*) handle, AS_JY_WITH_GC((PyObject*) handle), index, op, AS_JY(op));
	//updateJyGCHeadLinks((PyObject*) handle, AS_JY_WITH_GC((PyObject*) handle));
	return old;
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_add
 * Signature: (JILorg/python/core/PyObject;J)V
 */
void JyList_add(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj)
{
//	jputs(__FUNCTION__);
//	jputsLong(handle);
//	jputsLong(index);
	PyList_Append((PyObject*) handle, pyObj != NULL ? (PyObject*) pyObj : JyNI_PyObject_FromJythonPyObject(obj));
}

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_remove
 * Signature: (JI)Lorg/python/core/PyObject;
 */
jobject JyList_remove(JNIEnv *env, jclass class, jlong handle, jint index)
{
//	jputs(__FUNCTION__);
//	jputsLong(handle);
//	jputsLong(index);
	//list_ass_slice(self, i, i+1, (PyObject *)NULL)
	PyList_SetSlice((PyObject*) handle, index, index+1, (PyObject *) NULL);
}
