/*
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

package JyNI;

import org.python.core.Py;
import org.python.core.PyObject;
import org.python.core.PyString;
import org.python.core.PyType;
import org.python.core.Untraversable;

@Untraversable
public class PyShadowString extends PyString {
	public static final PyType TYPE = PyType.fromClass(PyShadowString.class);
//	private static PyString nativePlatform = new PyString(JyNI.getNativePlatform());

	/**
	 * The shadow-string is additionally used for comparisons, especially for __eq__.
	 * __eq__ will evaluate positive if the other string equals the actual value
	 * *or* the shadow.
	 */
	protected String shadow;

	public PyShadowString(String str, String shadow) {
		super(TYPE, str);
		this.shadow = shadow;
	}

	public PyShadowString(PyObject str, String shadow) {
		super(str.toString());
		this.shadow = shadow;
	}

	public PyShadowString(PyType subtype, String str, String shadow) {
		super(subtype, str);
		this.shadow = shadow;
	}

	public PyShadowString(PyType subtype, PyObject str, String shadow) {
		super(subtype, str.toString());
		this.shadow = shadow;
	}

	@Override
	public PyObject __eq__(PyObject other) {
//		(version with shadow being a PyString)
//		if (shadow.__eq__(other).__nonzero__()) return Py.True;
		String s = other.toString();
		if (s != null && s.equals(shadow)) return Py.True;
		return super.__eq__(other);
	}

// These overrides are currently ignored by Jython:
// (the versions seen here require shadow to be a PyString)
//
//	@Override
//	public boolean startswith(PyObject prefix) {
//		System.out.println("startswith1");
//		if (shadow.startswith(prefix)) return true;
//        return super.startswith(prefix);
//    }
//
//	@Override
//    public boolean startswith(PyObject prefix, PyObject start) {
//		System.out.println("startswith2");
//		if (shadow.startswith(prefix, start)) return true;
//        return super.startswith(prefix, start);
//    }
//
//    @Override
//    public boolean startswith(PyObject prefix, PyObject start, PyObject end) {
//    	System.out.println("startswith3");
//    	if (shadow.startswith(prefix, start, end)) return true;
//        return super.startswith(prefix, start, end);
//    }


// Legacy variant specifically for os.name:
//	@Override
//	public PyObject __eq__(PyObject other) {
//		String s = other.toString();
//		if (s != null) {
//			if (s.equals("posix")) return JyNI.isPosix() ? Py.True : Py.False;
//			if (s.equals("ce"))
//				return System.getProperty("os.name").equals("Windows CE") ?
//						Py.True : Py.False;
//			if (s.equals("nt"))
//				return System.getProperty("os.name").startsWith("Windows") ?
//						Py.True : Py.False;
//			if (s.equals("os2"))
//				return System.getProperty("os.name").toLowerCase().indexOf("os/2")
//						!= -1 ? Py.True : Py.False;
//			if (s.equals("riscos"))
//				return System.getProperty("os.name").toLowerCase().indexOf("risc")
//						!= -1 ? Py.True : Py.False;
//		}
//		return super.__eq__(other);
//	}
}
