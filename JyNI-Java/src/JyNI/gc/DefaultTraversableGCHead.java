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

import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Collection;

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
	@Override
	public void setLinks(Object links) {
//		if (links instanceof Iterable)
//			System.out.println(this.getClass()+".setLinks ("+System.identityHashCode(this)+") "+links.getClass()+" ("+System.identityHashCode(links)+")");
		gclinks = links;
	}

	@Override
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

	@Override
	public int setLink(int index, JyGCHead link) {
		//System.out.println(this.getClass()+".setLink ("+System.identityHashCode(this)+") "+index);
		int result = setLink(gclinks, index, link);
		if (result == 1) {
			gclinks = link;
			return 0;
		} else
			return result;
	}

	/**
	 * The result-value 1 tells the caller it shall perform
	 * links = link.
	 * This is needed, because this helper method cannot
	 * do this step for the caller.
	 */
	public static int setLink(Object links, int index, JyGCHead link) {
//		System.out.println(".setLink ("+System.identityHashCode(link)+") "+index);
//		System.out.println(index);
		if (links == null && index != 0) return -1;
		if (links instanceof JyGCHead[]) {
			JyGCHead[] ar = (JyGCHead[]) links;
			if (index >= ar.length) return -1;
			ar[index] = link;
			return 0;
		} else if (links == null || links instanceof JyGCHead) {
			/* Note: This case must be tested before check for Iterable, i.e.
			 * var-length case to avoid misinterpretation if links is both
			 * JyGCHead and Iterable (e.g. JyList). In this case JyGCHead
			 * comes first. The var-length case must always ground on an
			 * iterable data structure that is not a JyGCHead itself!
			 * So far we use a plain array-list there.
			 */
			if (index != 0) return -1;
			//result-value 1 tells the caller to set links = link
			return 1;
		} else if (links instanceof Iterable) {
			if (links instanceof List) {
				while (index >= ((List) links).size())
					((List) links).add(null);
				((List<JyGCHead>) links).set(index, link);
				return 0;
			} else if (links instanceof Collection) {
				int result = clearLink(links, index);
				if (result == 0)
					((Collection<JyGCHead>) links).add(link);
				return result;
			} else throw new UnsupportedOperationException();
		} else return -3;
	}

	@Override
	public int insertLink(int index, JyGCHead link) {
		return insertLink(gclinks, index, link);
	}

	public static int insertLink(Object links, int index, JyGCHead link) {
		if (links == null) return -2;
		if (links instanceof List) {
			if (index < 0 || index > ((List<JyGCHead>) links).size())
				return -1;
			((List<JyGCHead>) links).add(index, link);
			return 0;
		} else throw new UnsupportedOperationException();
	}

	@Override
	public int clearLink(int index) {
		return clearLink(gclinks, index);
	}

	public static int clearLink(Object links, int index) {
		if (links == null) return -2;
		if (links instanceof JyGCHead[]) {
			JyGCHead[] ar = (JyGCHead[]) links;
			if (index >= ar.length) return -1;
			ar[index] = null;
			return 0;
		} else if (links instanceof Iterable) {
			if (links instanceof List) {
				List<JyGCHead> ls = (List<JyGCHead>) links;
				if (index >= ls.size()) return -1;
				ls.remove(index);
				return 0;
			} else {
				Iterator<JyGCHead> ar = ((Iterable<JyGCHead>) links).iterator();
				for (int i = 0; i < index; ++i) {
					if (!ar.hasNext()) return -1;
					else ar.next();
				}
				if (!ar.hasNext()) return -1;
				else {
					ar.next();
					ar.remove();
					return 0;
				}
			}
		} else if (links instanceof JyGCHead) {
			if (index > 0) return -1;
			links = null;
			return 0;
		} else return -3;
	}

	@Override
	public int clearLinksFromIndex(int startIndex) {
		return clearLinksFromIndex(gclinks, startIndex);
	}

	public static int clearLinksFromIndex(Object links, int startIndex) {
		if (links == null) return -2;
		if (links instanceof JyGCHead[]) {
			JyGCHead[] ar = (JyGCHead[]) links;
			if (startIndex >= ar.length) return -1;
			for (int i = startIndex; i < ar.length; ++i)
				ar[i] = null;
			return ar.length-startIndex;
		} else if (links instanceof Iterable) {
			if (links instanceof List) {
				List<JyGCHead> ls = (List<JyGCHead>) links;
				if (startIndex >= ls.size()) return -1;
				int result = ls.size()-startIndex;
				for (int i = 0; i < result; ++i)
				/* For various list-implementations removing the last element is
				 * typically cheaper than removing in the middle:
				 */
					ls.remove(ls.size()-1);
				return result;
			} else {
				Iterator<JyGCHead> ar = ((Iterable<JyGCHead>) links).iterator();
				for (int i = 0; i < startIndex; ++i) {
					if (!ar.hasNext()) return -1;
					else ar.next();
				}
				int result = 0;
				while (ar.hasNext()) {
					++result;
					ar.next();
					ar.remove();
				}
				return result;
			}
		} else if (links instanceof JyGCHead) {
			if (startIndex > 0) return -1;
			links = null;
			return 1;
		} else return -3;
	}

	@Override
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

	@Override
	public long[] toHandleArray() {
		return toHandleArray(gclinks);
	}

	public static long[] toHandleArray(Object links) {
		if (links != null) {
			if (links instanceof JyGCHead[]) {
				JyGCHead[] ar = (JyGCHead[]) links;
				long[] result = new long[ar.length];
				for (int i = 0; i < ar.length; ++i) {
					if (ar[i] != null) {
						result[i] = ar[i].getHandle();
					}
				}
				return result;
			} else if (links instanceof Iterable) {
				Collection<JyGCHead> col;
				if (links instanceof Collection) {
					col = (Collection<JyGCHead>) links;
				} else {
					col = new ArrayList<JyGCHead>();
					Iterable<JyGCHead> ar = (Iterable<JyGCHead>) links;
					for (JyGCHead h: ar) {
						col.add(h);
					}
				}
				long[] result = new long[col.size()];
				int pos = 0;
				for (JyGCHead h: col) {
					if (h != null) {
						result[pos++] = h.getHandle();
					} else
						++pos;
				}
				return result;
			} else if (links instanceof JyGCHead) {
				long[] result = new long[1];
				result[0] = ((JyGCHead) links).getHandle();
				return result;
			}
		}
		return null;
	}

	@Override
	public void ensureSize(int size) {
		gclinks = ensureSize(gclinks, size);
	}

//	public static Object ensureSize(Object links, int size) {
//		Object result = ensureSize0(links, size);
//		System.out.println("After ensure:");
//		printLinksAsHashes(result, System.out);
//		return result;
//	}

	public static Object ensureSize(Object links, int size) {
		if (links == null || links instanceof JyGCHead)
		{
			if (size > 1) {
				JyGCHead[] result = new JyGCHead[size];
				result[0] = (JyGCHead) links;
				return result;
			} else if (size == 1) return links;
			else return null;
		} else if (links instanceof JyGCHead[]) {
			if (size > ((JyGCHead[]) links).length) {
				JyGCHead[] result = new JyGCHead[size];
				for (int i = 0; i < ((JyGCHead[]) links).length; ++i)
					result[i] = ((JyGCHead[]) links)[i];
				return result;
			} else return links;
		} else if (links instanceof Collection) {
			while (((Collection<JyGCHead>) links).size() < size)
				((Collection<JyGCHead>) links).add(null);
			return links;
		} else throw new IllegalArgumentException("links must be JyGCHead, JyGCHead[] or Collection<JyGCHead>.");
	}

	public static void printLinksAsHashes(Object links, java.io.PrintStream out) {
		if (links == null)
			out.println("no links");
		else if (links instanceof JyGCHead)
		{
			out.println(System.identityHashCode(links));
		} else if (links instanceof JyGCHead[]) {
			for (JyGCHead link: ((JyGCHead[]) links))
				out.println(link == null ? "null-link" : System.identityHashCode(link));
		} else if (links instanceof Collection) {
			for (JyGCHead link: ((Iterable<JyGCHead>) links))
				out.println(link == null ? "null-link" : System.identityHashCode(link));
		}
	}

	public void printLinks(java.io.PrintStream out) {
		printLinksAsHashes(gclinks, out);
	}
}
