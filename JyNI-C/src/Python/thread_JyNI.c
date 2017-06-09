/*
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

/*
 * thread_JyNI.c
 *
 *  Created on: 09.11.2013
 *      Author: Stefan Richthofer
 */

#include <JyNI.h>
#include <pythread_JyNI.h>

//PyAPI_FUNC(void) PyThread_init_thread(void);
static void PyThread__init_thread(void)
{

}

//PyAPI_FUNC(long) PyThread_start_new_thread(void (*)(void *), void *)
long PyThread_start_new_thread(void (*func) (void *), void *arg)
{
	//todo: Implement
	return 0;
}

PyAPI_FUNC(void) PyThread_exit_thread(void)
{
	//todo: Implement
}

PyAPI_FUNC(long) PyThread_get_thread_ident(void)
//long PyThread_get_thread_ident(void)
{
	env(0);
	return (long) (*env)->CallStaticLongMethod(env, JyNIClass, JyNI_getCurrentThreadID);
}

PyAPI_FUNC(PyThread_type_lock) PyThread_allocate_lock(void)
//PyThread_type_lock PyThread_allocate_lock(void)
{
	env(NULL);
	return (PyThread_type_lock) (*env)->NewGlobalRef(env, (*env)->NewObject(env, JyLockClass, JyLock_Constructor));
}

PyAPI_FUNC(void) PyThread_free_lock(PyThread_type_lock lock)
{
	env();
	(*env)->DeleteGlobalRef(env, (jobject) lock);
}

PyAPI_FUNC(int) PyThread_acquire_lock(PyThread_type_lock lock, int waitflag)
{
	env(0);
	return (*env)->CallBooleanMethod(env, (jobject) lock, JyLock_acquire, waitflag);//waitflag == WAIT_LOCK);
}

PyAPI_FUNC(void) PyThread_release_lock(PyThread_type_lock lock)
{
	env();
	(*env)->CallVoidMethod(env, (jobject) lock, JyLock_release);
	if ((*env)->ExceptionOccurred(env))
		(*env)->ExceptionClear(env); //this means, lock was already released, but we don't mind this
}
