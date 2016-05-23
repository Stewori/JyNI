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
 * JyList.h
 *
 *  Created on: 16.05.2013, 17:52:09
 *      Author: Stefan Richthofer
 */

#ifndef JYLIST_H_
#define JYLIST_H_

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_get
 * Signature: (JI)Lorg/python/core/PyObject;
 */
jobject JyList_get(JNIEnv *env, jclass class, jlong handle, jint index);

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_size
 * Signature: (J)I
 */
jint JyList_size(JNIEnv *env, jclass class, jlong handle);

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_set
 * Signature: (JILorg/python/core/PyObject;J)Lorg/python/core/PyObject;
 */
jobject JyList_set(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj);

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_add
 * Signature: (JILorg/python/core/PyObject;J)V
 */
void JyList_add(JNIEnv *env, jclass class, jlong handle, jint index, jobject obj, jlong pyObj);

/*
 * Class:     JyNI_JyNI
 * Method:    JyList_remove
 * Signature: (JI)Lorg/python/core/PyObject;
 */
jobject JyList_remove(JNIEnv *env, jclass class, jlong handle, jint index);

#endif /* JYLIST_H_ */
