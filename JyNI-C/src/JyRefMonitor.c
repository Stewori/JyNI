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
		size_t size, const char* type, const char* function, const char* file, jint line)
{
	if (op) assert(op == FROM_JY(object));
	JyRefMonitor_addAction2(action, object, NULL, size, type, function, file, line);
}

void JyRefMonitor_addAction2(jshort action, JyObject* object, JyObject* object2, size_t size,
		const char* type, const char* function, const char* file, jint line)
{
	env();
	jobject nativeAction = (*env)->NewObject(env, NativeActionClass, NativeAction_constructor);
	(*env)->SetShortField(env, nativeAction, NativeAction_actionField, action);
	(*env)->SetIntField(env, nativeAction, NativeAction_cLineField, line);

	(*env)->SetLongField(env, nativeAction, NativeAction_nativeRef1Field, (jlong) FROM_JY(object));

	if (object2)
		(*env)->SetLongField(env, nativeAction, NativeAction_nativeRef2Field, (jlong) FROM_JY(object));

	if (type)
	{
		jstring jtp = (*env)->NewStringUTF(env, type);
		(*env)->SetObjectField(env, nativeAction, NativeAction_cTypeNameField, jtp);
		(*env)->DeleteLocalRef(env, jtp);
	}

	if (function)
	{
		jstring jcm = (*env)->NewStringUTF(env, function);
		(*env)->SetObjectField(env, nativeAction, NativeAction_cMethodField, jcm);
		(*env)->DeleteLocalRef(env, jcm);
	}

	if (file)
	{
		jstring jfl = (*env)->NewStringUTF(env, file);
		(*env)->SetObjectField(env, nativeAction, NativeAction_cFileField, jfl);
		(*env)->DeleteLocalRef(env, jfl);
	}

	if (object && (object->flags & JY_INITIALIZED_FLAG_MASK))
		(*env)->SetObjectField(env, nativeAction, NativeAction_objField, object->jy);
	(*env)->CallStaticVoidMethod(env, JyReferenceMonitorClass, JyReferenceMonitor_addNativeAction, nativeAction);
	(*env)->DeleteLocalRef(env, nativeAction);
}
