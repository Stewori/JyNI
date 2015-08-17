/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
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

/**
 * Intended for use with native non-GC PyObjects in GIL-free mode.
 *
 * @author Stefan Richthofer
 */
public class SimpleGCHead implements JyGCHead {//PyObjectGCHead {
	protected long handle;
	//protected PyObject object;

	public SimpleGCHead(long handle) {
		this.handle = handle;
	}

	@Override
	public long getHandle() {
		return handle;
	}

	/*
	public SimpleGCHead(long handle, PyObject object) {
		this.handle = handle;
		this.object = object;
	}

	public PyObject getPyObject() {
		return object;
	}*/
}
