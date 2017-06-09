/* This File is based on sysmodule.h from CPython 2.7.3 release.
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


/* System module interface */

#ifndef Py_SYSMODULE_H
#define Py_SYSMODULE_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_FUNC(PyObject *) PySys_GetObject(char *);
PyAPI_FUNC(int) PySys_SetObject(char *, PyObject *);
PyAPI_FUNC(FILE *) PySys_GetFile(char *, FILE *);
PyAPI_FUNC(void) PySys_SetArgv(int, char **);
PyAPI_FUNC(void) PySys_SetArgvEx(int, char **, int);
PyAPI_FUNC(void) PySys_SetPath(char *);

PyAPI_FUNC(void) PySys_WriteStdout(const char *format, ...)
			Py_GCC_ATTRIBUTE((format(printf, 1, 2)));
PyAPI_FUNC(void) PySys_WriteStderr(const char *format, ...)
			Py_GCC_ATTRIBUTE((format(printf, 1, 2)));

PyAPI_DATA(PyObject *) _PySys_TraceFunc, *_PySys_ProfileFunc;
PyAPI_DATA(int) _PySys_CheckInterval;

PyAPI_FUNC(void) PySys_ResetWarnOptions(void);
PyAPI_FUNC(void) PySys_AddWarnOption(char *);
PyAPI_FUNC(int) PySys_HasWarnOptions(void);

#ifdef __cplusplus
}
#endif
#endif /* !Py_SYSMODULE_H */
