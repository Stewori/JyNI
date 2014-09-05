/*
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014 Python Software Foundation.  All rights reserved.
 * 
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014 Stefan Richthofer.  All rights reserved.
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

package JyNI;

import java.lang.reflect.Field;
import java.util.List;
import java.util.Set;
import java.util.Collection;
import java.util.Iterator;
import org.python.core.BaseSet;

import org.python.core.PyObject;

public class JySet implements Set<PyObject> {
	protected Set<PyObject> backend;
	long nativeBackendHandle;
	
	public JySet(long nativeBackendHandle)
	{
		this.nativeBackendHandle = nativeBackendHandle;
	}
	
	public JySet(Set<PyObject> backend, long nativeBackendHandle)
	{
		this.backend = backend;
		this.nativeBackendHandle = nativeBackendHandle;
	}
	
	public void installToPySet(BaseSet set)
	{
		try
		{
			Field backend = BaseSet.class.getDeclaredField("_set");
			backend.setAccessible(true);
			Set<PyObject> er = (Set<PyObject>) backend.get(set);
			if (this.backend == null)
				this.backend = er;
			backend.set(set, this);
		} catch (Exception e)
		{
			System.err.println("Problem modifying PySet backend: "+e);
		}
	}
	
	public Set<PyObject> installToPySetAndGetOldBackend(BaseSet set)
	{
		try
		{
			Field backend = BaseSet.class.getDeclaredField("_set");
			backend.setAccessible(true);
			Set<PyObject> er = (Set<PyObject>) backend.get(set);
			if (this.backend == null)
				this.backend = er;
			backend.set(set, this);
			return er;
		} catch (Exception e)
		{
			System.err.println("Problem in installToPySetAndGetOldBackend: "+e);
			return null;
		}
	}
	
	public boolean add(PyObject e) {
		if (backend.add(e)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public boolean addAll(Collection<? extends PyObject> c) {
		if (backend.addAll(c)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public void clear() {
		backend.clear();
		JyNI.JySet_putSize(nativeBackendHandle, 0);
	}
	
	public boolean contains(Object o) {return backend.contains(o);}
	public boolean containsAll(Collection<?> c) {return backend.containsAll(c);}
	public boolean equals(Object o) {return backend.equals(o);}
	public int hashCode() {return backend.hashCode();}
	public boolean isEmpty() {return backend.isEmpty();}
	public Iterator<PyObject> iterator() {return backend.iterator();}
	
	public boolean remove(Object o) {
		if (backend.remove(o)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public boolean removeAll(Collection<?> c) {
		if (backend.removeAll(c)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public boolean retainAll(Collection<?> c) {
		if (backend.retainAll(c)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public int size() {return backend.size();}
	public Object[] toArray() {return backend.toArray();}
	public <T> T[] toArray(T[] a) {return backend.toArray(a);}
}
