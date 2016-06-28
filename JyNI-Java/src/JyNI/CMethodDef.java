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

package JyNI;

import org.python.core.*;


/**
 * To map native MethodDescriptor objects to Jython we need this simple
 * PyBuiltinCallable-implementation. Binding works by a native call.
 * This implementation is special in the sense that Java-side is a
 * PyObject while C-side - PyMethodDef - is not. We treat it mostly
 * like Java-side also wasn't and once a method is bound, we use
 * PyCFunction instead.
 * PyCFunction is also a PyBuiltinCallable, but stands for a native
 * PyCFunctionObject, which is an actual PyObject too.
 *
 * @author Stefan Richthofer
 */
public class CMethodDef extends PyBuiltinCallable {

	public long handle;

	public CMethodDef(long handle, String name, boolean no_args, String doc) {
		super(new DefaultInfo(name, 0, no_args ? 0 : -1));
		this.handle = handle;
		this.doc = doc;
	}

	@Override
	public PyBuiltinCallable bind(PyObject bindTo) {
		System.out.println("CmethodDef.bind: "+this.info.getName());
		System.out.println(bindTo);
		PyObject er = JyNI.maybeExc(JyNI.JyNI_CMethodDef_bind(handle, bindTo,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		return (PyBuiltinCallable) er;
	}
}
