/* This File is based on pydebug.h from CPython 2.7.3 release.
 * It has been modified to suit JyNI needs.
 *
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
 * 2010, 2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
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


#ifndef Py_PYDEBUG_H
#define Py_PYDEBUG_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_DATA(int) Py_DebugFlag;
PyAPI_DATA(int) Py_VerboseFlag;
PyAPI_DATA(int) Py_InteractiveFlag;
PyAPI_DATA(int) Py_InspectFlag;
PyAPI_DATA(int) Py_OptimizeFlag;
PyAPI_DATA(int) Py_NoSiteFlag;
PyAPI_DATA(int) Py_BytesWarningFlag;
PyAPI_DATA(int) Py_UseClassExceptionsFlag;
PyAPI_DATA(int) Py_FrozenFlag;
PyAPI_DATA(int) Py_TabcheckFlag;
PyAPI_DATA(int) Py_UnicodeFlag;
PyAPI_DATA(int) Py_IgnoreEnvironmentFlag;
PyAPI_DATA(int) Py_DivisionWarningFlag;
PyAPI_DATA(int) Py_DontWriteBytecodeFlag;
PyAPI_DATA(int) Py_NoUserSiteDirectory;
/* _XXX Py_QnewFlag should go away in 3.0.  It's true iff -Qnew is passed,
  on the command line, and is used in 2.2 by ceval.c to make all "/" divisions
  true divisions (which they will be in 3.0). */
PyAPI_DATA(int) _Py_QnewFlag;
/* Warn about 3.x issues */
PyAPI_DATA(int) Py_Py3kWarningFlag;
PyAPI_DATA(int) Py_HashRandomizationFlag;

/* this is a wrapper around getenv() that pays attention to
   Py_IgnoreEnvironmentFlag.  It should be used for getting variables like
   PYTHONPATH and PYTHONHOME from the environment */
#define Py_GETENV(s) (Py_IgnoreEnvironmentFlag ? NULL : getenv(s))

PyAPI_FUNC(void) Py_FatalError(const char *message);

#ifdef __cplusplus
}
#endif
#endif /* !Py_PYDEBUG_H */
