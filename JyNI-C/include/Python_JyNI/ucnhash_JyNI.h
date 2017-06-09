/* This File is based on ucnhash.h from CPython 2.7.10 release.
 * It has been modified to suit JyNI needs.
 *
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
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


/* Unicode name database interface */

#ifndef Py_UCNHASH_H
#define Py_UCNHASH_H
#ifdef __cplusplus
extern "C" {
#endif

/* revised ucnhash CAPI interface (exported through a "wrapper") */

#define PyUnicodeData_CAPSULE_NAME "unicodedata.ucnhash_CAPI"

typedef struct {

    /* Size of this struct */
    int size;

    /* Get name for a given character code.  Returns non-zero if
       success, zero if not.  Does not set Python exceptions. 
       If self is NULL, data come from the default version of the database.
       If it is not NULL, it should be a unicodedata.ucd_X_Y_Z object */
    int (*getname)(PyObject *self, Py_UCS4 code, char* buffer, int buflen);

    /* Get character code for a given name.  Same error handling
       as for getname. */
    int (*getcode)(PyObject *self, const char* name, int namelen, Py_UCS4* code);

} _PyUnicode_Name_CAPI;

#ifdef __cplusplus
}
#endif
#endif /* !Py_UCNHASH_H */
