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

public interface TraversableGCHead extends JyGCHead {
	/**
	 * Do not call this method. It is for internal use and only
	 * public because interfaces require this.
	 * 
	 * Note:
	 * links must be of one of these types:
	 * JyGCHead, JyGCHead[] or Iterable<JyGCHead>
	 */
	public void setLinks(Object links);

	/**
	 * Do not call this method. It is for internal use and only
	 * public because interfaces require this.
	 *
	 * In var-size case this operation is only supported if the
	 * backend is a Collection. (Since an arbitrary iterable has
	 * no add- or set-operation.)
	 *
	 * @return -2 if links == null, -1 for invalid index, 0 otherwise.
	 */
	public int setLink(int index, JyGCHead link);

	/**
	 * Do not call this method. It is for internal use and only
	 * public because interfaces require this.
	 *
	 * In var-size case this operation is only supported if the
	 * backend is a List. In other cases it is not supported at all.
	 *
	 * @return -2 if links == null, -1 for invalid index, 0 otherwise.
	 */
	public int insertLink(int index, JyGCHead link);
	
	/**
	 * Do not call this method. It is for internal use and only
	 * public because interfaces require this.
	 *
	 * Removes the indexed element from GC's responsibility.
	 * In var-size case, this actually removes the element.
	 * In fixed-size case this sets the element-position to null.
	 * In single-link case it sets the links-variable to null.
	 *
	 * @return -2 if links == null, -1 for invalid index, 0 otherwise.
	 */
	public int clearLink(int index);
	
	/**
	 * Do not call this method. It is for internal use and only
	 * public because interfaces require this.
	 *
	 * Removes the all elements with index >= startIndex from GC's responsibility.
	 * In var-size case, this actually removes the elements.
	 * In fixed-size case this sets the element-positions to null.
	 * In single-link case it sets the links-variable to null.
	 *
	 * @return -2 if links == null, -1 for invalid startIndex, number of cleared indices otherwise.
	 */
	public int clearLinksFromIndex(int startIndex);
	public int jyTraverse(JyVisitproc visit, Object arg);

	public long[] toHandleArray();
}
