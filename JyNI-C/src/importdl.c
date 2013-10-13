/* This File is based on importdl.c from CPython 2.7.3 release.
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

	//The following case is covered on java-side now:
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
	//puts("got dyn load func");
	if (PyErr_Occurred())
	{
		//puts("PyErrOccured00");
		return NULL;
	}
	//puts("error check done");
	env(NULL);
	if (p == NULL) {
		//puts("no init function");
		JyNI_JyErr_Format((*env)->GetStaticObjectField(env, pyPyClass, pyPyImportError),
		   "dynamic module does not define init function (init%.200s)",
					 shortname);
		return NULL;
	}
	//puts("dyn load func is not NULL");
	oldcontext = _Py_PackageContext;
	_Py_PackageContext = packagecontext;
	(*p)();
	_Py_PackageContext = oldcontext;
	if (PyErr_Occurred())
	{
		//puts("return NULL because PyErr_Occurred");
		return NULL;
	}

	//m = PyDict_GetItemString(PyImport_GetModuleDict(), name);
	//puts("retrieving module...");
	//puts(name);
	//the following somehow goes wrong, probaby because we didn't call String.intern...
	/*jobject mName = (*env)->CallStaticObjectMethod(env, pyPyClass, pyPyNewString, (*env)->NewStringUTF(env, name));
	m = (*env)->CallObjectMethod(env,
			(*env)->CallStaticObjectMethod(env, JyNIClass, JyNIPyImport_GetModuleDict),
			pyDictGet_PyObject,
			mName
		);*/
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
	if (PyModule_AddStringConstantJy(m, "__file__", pathname) < 0)
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
	//puts("importdl done");
	return m;
}

//#endif // HAVE_DYNAMIC_LOADING
