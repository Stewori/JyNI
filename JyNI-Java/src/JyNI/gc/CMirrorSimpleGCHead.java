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


package JyNI.gc;

import java.lang.ref.WeakReference;
import org.python.core.PyObject;

public class CMirrorSimpleGCHead extends SimpleGCHead implements PyObjectGCHead {
	//SoftReference<PyObject> object; //For now use WeaReference for easier debugging.
	WeakReference<PyObject> object; //Later change this to SoftReference

	public CMirrorSimpleGCHead(long handle) {
		super(handle);
	}

	public void setPyObject(PyObject object) {
		//this.object = new SoftReference<>(object);
		this.object = new WeakReference<>(object);
	}

	public PyObject getPyObject() {
//		if (object == null) {
//			System.out.println("JyNI-Warning: In CMirrorSimpleGCHead object not set!");
//			System.out.println(handle+" ("+JyNI.JyNI.getNativeTypeName(handle)+")");
//		}
		return object != null ? object.get() : null;
	}
}
