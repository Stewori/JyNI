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


package JyNI.gc;

public class DefaultTraversableGCHead implements TraversableGCHead {
	protected Object gclinks;
	protected long handle;

	public DefaultTraversableGCHead(long handle) {
		//super(handle);
		gclinks = null;
		this.handle = handle;
	}

//	public DefaultTraversableGCHead(long handle, PyObject object) {
//		super(handle, object);
//		gclinks = null;
//	}

	/**
	 * Do not call this method. It is for internal use and only public
	 * to implement an interface.
	 */
	public void setLinks(Object links) {
		gclinks = links;
	}

	public long getHandle() {
		return handle;
	}

	/*
	public DefaultTraversableGCHead(JyGCHead[] links, long handle) {
		super(handle);
		this.links = links;
	}

	public DefaultTraversableGCHead(Iterable<JyGCHead> links, long handle) {
		super(handle);
		this.links = links;
	}

	public DefaultTraversableGCHead(JyGCHead link, long handle) {
		super(handle);
		this.links = link;
	}*/

	public int jyTraverse(JyVisitproc visit, Object arg) {
		return jyTraverse(gclinks, visit, arg);
	}

	public static int jyTraverse(Object links, JyVisitproc visit, Object arg) {
		if (links != null) {
			if (links instanceof JyGCHead[]) {
				int result = 0;
				JyGCHead[] ar = (JyGCHead[]) links;
				for (JyGCHead h: ar) {
					if (h != null) {
						result = visit.jyVisit(h, arg);
						if (result != 0) return result;
					}
				}
				return result;
			} else if (links instanceof Iterable) {
				int result = 0;
				Iterable<JyGCHead> ar = (Iterable<JyGCHead>) links;
				for (JyGCHead h: ar) {
					if (h != null) {
						result = visit.jyVisit(h, arg);
						if (result != 0) return result;
					}
				}
				return result;
			} else if (links instanceof JyGCHead) {
				return visit.jyVisit((JyGCHead) links, arg);
			}
		}
		return 0;
	}
}
