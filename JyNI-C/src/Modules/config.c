/* This File is based on config.c from CPython 2.7.13 release.
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

/* Module configuration */

/* This file contains the table of built-in modules.
   See init_builtin() in import.c. */

#include "JyNI.h"

#ifdef WIN32
extern void initmsvcrt(void);
//extern void init_locale(void);
#endif
//extern void init_codecs(void);
//extern void init_weakref(void);
//extern void init_hotshot(void);
//extern void initxxsubtype(void);
//extern void initzipimport(void);
//extern void init_random(void);
//extern void inititertools(void);
//extern void init_collections(void);
//extern void init_heapq(void);
//extern void init_bisect(void);
//extern void init_symtable(void);
#ifdef MS_WINDOWS
extern void initmmap(void);
extern void init_winreg(void);
#endif
//extern void init_struct(void);
extern void initdatetime(void);
//extern void init_functools(void);
//extern void init_json(void);
//extern void initzlib(void);
//
//extern void init_multibytecodec(void);
//extern void init_codecs_cn(void);
//extern void init_codecs_hk(void);
//extern void init_codecs_iso2022(void);
//extern void init_codecs_jp(void);
//extern void init_codecs_kr(void);
//extern void init_codecs_tw(void);
//extern void init_subprocess(void);
//extern void init_lsprof(void);
//extern void init_ast(void);
//extern void init_io(void);
//extern void _PyWarnings_Init(void);

/* tools/freeze/makeconfig.py marker for additional "extern" */
/* -- ADDMODULE MARKER 1 -- */

//extern void PyMarshal_Init(void);
//extern void initimp(void);

struct _inittab _PyImport_Inittab[] = {

//	{"array", initarray},
//	{"_ast", init_ast},
//#ifdef MS_WINDOWS
//#ifndef MS_WINI64
//	{"audioop", initaudioop},
//#endif
//#endif
//	{"binascii", initbinascii},
//	{"cmath", initcmath},
//	{"errno", initerrno},
//	{"future_builtins", initfuture_builtins},
//	{"gc", initgc},
//#ifndef MS_WINI64
//	{"imageop", initimageop},
//#endif
//	{"math", initmath},
//	{"_md5", init_md5},
//	{"nt", initnt}, /* Use the NT os functions, not posix */
//	{"operator", initoperator},
//	{"signal", initsignal},
//	{"_sha", init_sha},
//	{"_sha256", init_sha256},
//	{"_sha512", init_sha512},
//	{"strop", initstrop},
//	{"time", inittime},
//#ifdef WITH_THREAD
//	{"thread", initthread},
//#endif
//	{"cStringIO", initcStringIO},
//	{"cPickle", initcPickle},
#ifdef WIN32
	{"msvcrt", initmsvcrt},
//	{"_locale", init_locale},
#endif
//	/* XXX Should _subprocess go in a WIN32 block?  not WIN64? */
//	{"_subprocess", init_subprocess},
//
//	{"_codecs", init_codecs},
//	{"_weakref", init_weakref},
//	{"_hotshot", init_hotshot},
//	{"_random", init_random},
//	{"_bisect", init_bisect},
//	{"_heapq", init_heapq},
//	{"_lsprof", init_lsprof},
//	{"itertools", inititertools},
//	{"_collections", init_collections},
//	{"_symtable", init_symtable},
//	{"_csv", init_csv},
//	{"_sre", init_sre},
//	{"parser", initparser},
#ifdef MS_WINDOWS
	{"_winreg", init_winreg},
	{"mmap", initmmap},
#endif
//	{"_struct", init_struct},
	{"datetime", initdatetime},
//	{"_functools", init_functools},
//	{"_json", init_json},
//
//	{"xxsubtype", initxxsubtype},
//	{"zipimport", initzipimport},
//	{"zlib", initzlib},
//
//	/* CJK codecs */
//	{"_multibytecodec", init_multibytecodec},
//	{"_codecs_cn", init_codecs_cn},
//	{"_codecs_hk", init_codecs_hk},
//	{"_codecs_iso2022", init_codecs_iso2022},
//	{"_codecs_jp", init_codecs_jp},
//	{"_codecs_kr", init_codecs_kr},
//	{"_codecs_tw", init_codecs_tw},
//
///* tools/freeze/makeconfig.py marker for additional "_inittab" entries */
///* -- ADDMODULE MARKER 2 -- */
//
//	/* This module "lives in" with marshal.c */
//	{"marshal", PyMarshal_Init},
//
//	/* This lives it with import.c */
//	{"imp", initimp},
//
//	/* These entries are here for sys.builtin_module_names */
//	{"__main__", NULL},
//	{"__builtin__", NULL},
//	{"sys", NULL},
//	{"exceptions", NULL},
//	{"_warnings", _PyWarnings_Init},
//
//	{"_io", init_io},

	/* Sentinel */
	{0, 0}
};
