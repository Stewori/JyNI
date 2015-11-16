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

import JyNI.PyCPeerType;
import org.python.core.PyObject;
import org.python.core.Untraversable;
import org.python.core.PyType;

@Untraversable
public class PyCPeerTypeGC extends PyCPeerType implements TraversableGCHead {
	protected Object links;

	public PyCPeerTypeGC(long objectHandle) {
		super(objectHandle);
	}

	public PyCPeerTypeGC(long objectHandle, String name, PyObject dict) {
		super(objectHandle, name, dict);
	}

	public PyCPeerTypeGC(long objectHandle, String name, PyObject dict, PyType metatype) {
		super(objectHandle, name, dict, metatype);
	}

	public void setLinks(Object links) {
		this.links = links;
	}

	@Override
	public int setLink(int index, JyGCHead link) {
		return DefaultTraversableGCHead.setLink(links, index, link);
	}

	@Override
	public int insertLink(int index, JyGCHead link) {
		return DefaultTraversableGCHead.insertLink(links, index, link);
	}

	@Override
	public int clearLink(int index) {
		return DefaultTraversableGCHead.clearLink(links, index);
	}

	@Override
	public int clearLinksFromIndex(int startIndex) {
		return DefaultTraversableGCHead.clearLinksFromIndex(links, startIndex);
	}

	@Override
	public int jyTraverse(JyVisitproc visit, Object arg) {
		return DefaultTraversableGCHead.jyTraverse(links, visit, arg);
	}

	@Override
	public long[] toHandleArray() {
		return DefaultTraversableGCHead.toHandleArray(links);
	}
}
