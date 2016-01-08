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
 * JyRefMonitor.c
 *
 *  Created on: 20.05.2014
 *      Author: Stefan Richthofer
 */
#include <JyNI.h>

jint Jy_memDebugFlags = 0;

/*
 * Class:     JyNI_JyNI
 * Method:    JyRefMonitor_setMemDebug
 * Signature: (Z)V
 */
void JyRefMonitor_setMemDebugFlags(JNIEnv *env, jclass class, jint flags)
{
	Jy_memDebugFlags = flags;
//	if (flags) jputs("JyNI: memDebug enabled!");
//	else jputs("JyNI: memDebug disabled!");
}

void JyRefMonitor_addAction(jshort action, PyObject* op, JyObject* object,
		size_t size, char* type, char* function, char* file, jint line)
{
	if (op)
		assert(op == FROM_JY(object));
	env();
	jstring jtp = type == NULL ? NULL : (*env)->NewStringUTF(env, type);
	jstring jcm = function == NULL ? NULL : (*env)->NewStringUTF(env, function);
	jstring jfl = file == NULL ? NULL : (*env)->NewStringUTF(env, file);
	jweak jop = NULL;
	jstring repr = NULL;
	//if (PyString_CheckExact(FROM_JY(object))) jputsLong(__LINE__);
	//if (type && strcmp(type, "str")) jputsLong(__LINE__);
//		repr = (*env)->NewStringUTF(env, PyString_AS_STRING(FROM_JY(object)));
	if (object && (object->flags & JY_INITIALIZED_FLAG_MASK)) jop = object->jy;
	jobject dbgInfo = (*env)->CallStaticObjectMethod(env, JyReferenceMonitorClass,
			JyRefMonitorMakeDebugInfo, jtp, jcm, line, jfl);
	(*env)->CallStaticVoidMethod(env, JyReferenceMonitorClass,
			JyRefMonitorAddAction, action, jop, (jlong) FROM_JY(object), NULL,
			dbgInfo);//jtp, jcm, jfl, line, repr);
}

void JyRefMonitor_addAction2(jshort action, JyObject* object, JyObject* object2, size_t size,
		char* type, char* function, char* file, jint line)
{
	env();
	jstring jtp = type == NULL ? NULL : (*env)->NewStringUTF(env, type);
	jstring jcm = function == NULL ? NULL : (*env)->NewStringUTF(env, function);
	jstring jfl = file == NULL ? NULL : (*env)->NewStringUTF(env, file);
	jstring repr = NULL;
//	if (PyString_Check(FROM_JY(object)))
//		repr = (*env)->NewStringUTF(env, PyString_AS_STRING(FROM_JY(object)));
	jweak jop = NULL;
	if (object && (object->flags & JY_INITIALIZED_FLAG_MASK)) jop = object->jy;
	jobject dbgInfo = (*env)->CallStaticObjectMethod(env, JyReferenceMonitorClass,
			JyRefMonitorMakeDebugInfo, jtp, jcm, line, jfl);
	(*env)->CallStaticVoidMethod(env, JyReferenceMonitorClass,
			JyRefMonitorAddAction, action, jop, (jlong) FROM_JY(object), (jlong) FROM_JY(object2),
			dbgInfo);//jtp, jcm, jfl, line, repr);
}
