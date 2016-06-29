/* This File is based on importdl.c from CPython 2.7.3 release.
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


/* Support for dynamic loading of extension modules */

#include "JyNI.h"

/* ./configure sets HAVE_DYNAMIC_LOADING if dynamic loading of modules is
   supported on this platform. configure will then compile and link in one
   of the dynload_*.c files, as appropriate. We will call a function in
   those modules to get a function pointer to the module's init function.
*/
//#ifdef HAVE_DYNAMIC_LOADING

#include "importdl.h"

extern dl_funcptr _PyImport_GetDynLoadFunc(const char *name,
										   const char *shortname,
										   const char *pathname, FILE *fp);



PyObject *
_PyImport_LoadDynamicModule(char *name, char *pathname, FILE *fp)
{
	PyObject* m = JyNI_PyObject_FromJythonPyObject(_PyImport_LoadDynamicModuleJy(name, pathname, fp));
	Py_INCREF(m);
	return m;
}

jobject _PyImport_LoadDynamicModuleJy(char *name, char *pathname, FILE *fp)
{
	//PyObject *m;
	jobject m;
	char *lastdot, *shortname, *packagecontext, *oldcontext;
	dl_funcptr p;

	//The following case is covered on Java-side now:
	/*if ((m = _PyImport_FindExtension(name, pathname)) != NULL) {
		Py_INCREF(m);
		return m;
	}*/
	lastdot = strrchr(name, '.');
	if (lastdot == NULL) {
		packagecontext = NULL;
		shortname = name;
	}
	else {
		packagecontext = name;
		shortname = lastdot+1;
	}
	//PyErr_Clear(); //added temporarily by JyNI to focus on further development until the exception thing is solved.
	p = _PyImport_GetDynLoadFunc(name, shortname, pathname, fp);
	//jputs("got dyn load func");
	if (PyErr_Occurred())
	{
//		jputs("PyErrOccured00");
//		PyThreadState *tstate = PyThreadState_GET();
//		jputs(((PyStringObject*) tstate->curexc_value)->ob_sval);
		return NULL;
	}
	//jputs("error check done");
	if (p == NULL) {
		//jputs("no init function");
		//JyNI_JyErr_Format((*env)->GetStaticObjectField(env, pyPyClass, pyPyImportError),
		PyErr_Format(PyExc_ImportError,
		   "dynamic module does not define init function (init%.200s)",
			shortname);
		return NULL;
	}
	//jputs("dyn load func is not NULL");
	oldcontext = _Py_PackageContext;
	_Py_PackageContext = packagecontext;
//	jputs("run dyn load func...");
//	jputs(name);
	(*p)();
//	jputs("run dyn load func done");
//	jputs(name);
	_Py_PackageContext = oldcontext;
	if (PyErr_Occurred())
	{
//		jputs("PyErrOccured02");
//		PyThreadState *tstate = PyThreadState_GET();
//		jputs(((PyStringObject*) tstate->curexc_value)->ob_sval);
		return NULL;
	}

	//m = PyDict_GetItemString(PyImport_GetModuleDict(), name);
	//jputs("retrieving module...");
	//jputs(name);
	//the following somehow goes wrong, probaby because we didn't call String.intern...
	/*jobject mName = (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewString, (*env)->NewStringUTF(env, name));
	m = (*env)->CallObjectMethod(env,
			(*env)->CallStaticObjectMethod(env, JyNIClass, JyNIPyImport_GetModuleDict),
			pyDictGet_PyObject,
			mName
		);*/
	env(NULL);
	m = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNIJyNI_GetModule, (*env)->NewStringUTF(env, name));
	//puts("retrieved module");
	if (m == NULL) {
		//puts("m = NULL");
		PyErr_SetString(PyExc_SystemError,
						"dynamic module not initialized properly");
		return NULL;
	}
	//puts("name:");
	//puts(PyModule_GetName(JyNI_PyObject_FromJythonPyObject(m)));
	// Remember the filename as the __file__ attribute
	//if (PyModule_AddStringConstant(m, "__file__", pathname) < 0)
	//puts("adding filename...");
	// Todo: If pathname is NULL set __file__ to PyNone or something.
	if (pathname && PyModule_AddStringConstantJy(m, "__file__", pathname) < 0)
		PyErr_Clear(); // Not important enough to report
	//puts("filename added:");
	//puts(PyModule_GetFilename(JyNI_PyObject_FromJythonPyObject(m)));
	//provide FixupExtension later...
	//if (_PyImport_FixupExtension(name, pathname) == NULL)
	//	return NULL;
	//if (Py_VerboseFlag)
	if ((*env)->CallStaticIntMethod(env, JyNIClass, JyNIGetDLVerbose))
		PySys_WriteStderr(
			"import %s # dynamically loaded from %s\n",
			name, pathname);
	//Py_INCREF(m);
	//jputs("module loaded:");
	//jputs(name);
	//jputsLong(m);
	return m;
}

//#endif // HAVE_DYNAMIC_LOADING
