/* This File is based on osdefs.h from CPython 2.7.10 release.
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

#ifndef Py_OSDEFS_H
#define Py_OSDEFS_H
#ifdef __cplusplus
extern "C" {
#endif


/* Operating system dependencies */

/* Mod by chrish: QNX has WATCOM, but isn't DOS */
#if !defined(__QNX__)
#if defined(MS_WINDOWS) || defined(__BORLANDC__) || defined(__WATCOMC__) || defined(__DJGPP__) || defined(PYOS_OS2)
#if defined(PYOS_OS2) && defined(PYCC_GCC)
#define MAXPATHLEN 260
#define SEP '/'
#define ALTSEP '\\'
#else
#define SEP '\\'
#define ALTSEP '/'
#define MAXPATHLEN 256
#endif
#define DELIM ';'
#endif
#endif

#ifdef RISCOS
#define SEP '.'
#define MAXPATHLEN 256
#define DELIM ','
#endif


/* Filename separator */
#ifndef SEP
#define SEP '/'
#endif

/* Max pathname length */
#ifdef __hpux
#include <sys/param.h>
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX MAXPATHLEN
#endif
#endif

#ifndef MAXPATHLEN
#if defined(PATH_MAX) && PATH_MAX > 1024
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 1024
#endif
#endif

/* Search path entry delimiter */
#ifndef DELIM
#define DELIM ':'
#endif

#ifdef __cplusplus
}
#endif
#endif /* !Py_OSDEFS_H */
