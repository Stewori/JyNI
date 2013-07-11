/* This File is based on importdl.h from CPython 2.7.3 release.
 * It has been modified to suite JyNI needs.
 *
 * Copyright of the original file:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013 Stefan Richthofer.  All rights reserved.
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


#ifndef Py_IMPORTDL_H
#define Py_IMPORTDL_H

#ifdef __cplusplus
extern "C" {
#endif


/* Definitions for dynamic loading of extension modules */
enum filetype {
    SEARCH_ERROR,
    PY_SOURCE,
    PY_COMPILED,
    C_EXTENSION,
    PY_RESOURCE, /* Mac only */
    PKG_DIRECTORY,
    C_BUILTIN,
    PY_FROZEN,
    PY_CODERESOURCE, /* Mac only */
    IMP_HOOK
};

struct filedescr {
    char *suffix;
    char *mode;
    enum filetype type;
};
extern struct filedescr * _PyImport_Filetab;
extern const struct filedescr _PyImport_DynLoadFiletab[];

extern PyObject *_PyImport_LoadDynamicModule(char *name, char *pathname,
                                             FILE *);

/* Max length of module suffix searched for -- accommodates "module.slb" */
#define MAXSUFFIXSIZE 12

#ifdef MS_WINDOWS
#include <windows.h>
typedef FARPROC dl_funcptr;
#else
#if defined(PYOS_OS2) && !defined(PYCC_GCC)
#include <os2def.h>
typedef int (* APIENTRY dl_funcptr)();
#else
typedef void (*dl_funcptr)(void);
#endif
#endif


#ifdef __cplusplus
}
#endif
#endif /* !Py_IMPORTDL_H */
