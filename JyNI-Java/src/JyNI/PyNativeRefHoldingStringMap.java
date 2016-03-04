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

import java.util.AbstractSet;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentMap;

import org.python.core.AbstractDict;
import org.python.core.Py;
import org.python.core.PyObject;
import org.python.core.PyString;
import org.python.core.PyStringMap;
import org.python.core.PyTuple;

/**
 * For now we only hold native references for values
 * rather than also for keys.
 *
 * @author Stefan Richthofer
 */
public class PyNativeRefHoldingStringMap extends PyStringMap {

	public PyNativeRefHoldingStringMap() {
		this(4);
	}

	public PyNativeRefHoldingStringMap(int capacity) {
		super(capacity);
	}

	public PyNativeRefHoldingStringMap(Map<Object, PyObject> map) {
		super(map);
		for (PyObject obj: map.values())
			nativeIncref(obj);
	}

	public PyNativeRefHoldingStringMap(PyObject elements[]) {
		this(elements.length);
		for (int i = 0; i < elements.length; i += 2) {
			super.__setitem__(elements[i], elements[i + 1]);
			nativeIncref(elements[i + 1]);
		}
	}

	protected static void nativeIncref(PyObject obj) {
		long handle = JyNI.lookupNativeHandle(obj);
		if (handle != 0)
			JyNI.nativeIncref(handle,
					JyTState.prepareNativeThreadState(Py.getThreadState()));
	}

	protected static void nativeDecref(PyObject obj) {
		long handle = JyNI.lookupNativeHandle(obj);
		if (handle != 0)
			JyNI.nativeDecref(handle,
					JyTState.prepareNativeThreadState(Py.getThreadState()));
	}

	@Override
	public void __setitem__(String key, PyObject value) {
		PyObject old = __finditem__(key);
		nativeIncref(value);
		if (old != null) nativeDecref(old);
		super.__setitem__(key, value);
	}

	@Override
	public void __setitem__(PyObject key, PyObject value) {
		PyObject old = __finditem__(key);
		nativeIncref(value);
		if (old != null) nativeDecref(old);
		super.__setitem__(key, value);
	}

	@Override
	public void __delitem__(String key) {
		PyObject old = __finditem__(key);
		if (old != null) nativeDecref(old);
		super.__delitem__(key);
	}

	@Override
	public void __delitem__(PyObject key) {
		PyObject old = __finditem__(key);
		if (old != null) nativeDecref(old);
		super.__delitem__(key);
	}

	/**
	 * Remove all items from the dictionary.
	 */
	public void clear() {
		for (PyObject obj: getMap().values())
			nativeDecref(obj);
		super.clear();
	}


	/**
	 * Return a shallow copy of the dictionary.
	 */
	public PyNativeRefHoldingStringMap copy() {
		return new PyNativeRefHoldingStringMap(getMap());
	}

	/**
	 * Merge another PyObject that supports keys() with this
	 * dict.
	 *
	 * @param other a PyObject with a keys() method
	 * @param override if true, the value from other is used on key-collision
	 */
	public void merge(PyObject other, boolean override) {
		// Yes this is rather inefficient compared to original implementation.
		// Just don't use it on PyNativeRefHoldingStringMap ;)
		synchronized(getMap()) {
			if (other instanceof AbstractDict) {
				mergeFromKeys(other, ((AbstractDict) other).keys(), override);
			} else {
				mergeFromKeys(other, other.invoke("keys"), override);
			}
		}
	}

	/**
	 * Return this[key] if the key exist, otherwise insert key with the value of failobj and return
	 * failobj
	 *
	 * @param key
	 *			the key to lookup in the mapping.
	 * @param failobj
	 *			the default value to insert in the mapping if key does not already exist.
	 */
	public PyObject setdefault(PyObject key, PyObject failobj) {
		Object internedKey = (key instanceof PyString) ? ((PyString)key).internedString() : key;
		PyObject oldValue = getMap().putIfAbsent(internedKey, failobj);
		if (oldValue != null) nativeIncref(failobj);
		return oldValue == null ? failobj : oldValue;
	}

	/**
	 * Return a random (key, value) tuple pair and remove the pair from the mapping.
	 */
	public PyObject popitem() {
		PyTuple result = (PyTuple) super.popitem();
		nativeDecref(result.__getitem__(1));
		return result;
	}

	/* not correct - we need to determine size and remove at the same time! */
	public PyObject pop(PyObject key) {
		if (getMap().size() == 0) {
			throw Py.KeyError("pop(): dictionary is empty");
		}
		PyObject value = getMap().remove(pyToKey(key));
		if (value == null) {
			throw Py.KeyError(key);
		}
		nativeDecref(value);
		return value;
	}

	public PyObject pop(PyObject key, PyObject failobj) {
		PyObject value = getMap().remove(pyToKey(key));
		if (value == null) {
			if (failobj == null) {
				throw Py.KeyError(key);
			} else {
				return failobj;
			}
		}
		nativeDecref(value);
		return value;
	}

	// todo: Make this protected in Jython and remove here.
	private static Object pyToKey(PyObject pyKey) {
		if (pyKey instanceof PyString) {
			return ((PyString)pyKey).internedString();
		} else {
			return pyKey;
		}
	}

	// todo: Make this protected in Jython and remove here.
	private static PyObject keyToPy(Object objKey){
		if (objKey instanceof String) {
			return PyString.fromInterned((String)objKey);
		} else {
			return (PyObject)objKey;
		}
	}

	private static class PyNativeRefHoldingStringMapKeySetWrapper extends AbstractSet<PyObject>
	{
		Set<Object> backend;
		ConcurrentMap<Object, PyObject> backtable;

		PyNativeRefHoldingStringMapKeySetWrapper(ConcurrentMap<Object, PyObject> backtable,
				Set<Object> backend) {
			this.backend = backend;
			this.backtable = backtable;
		}

		class PyNativeRefHoldingStringMapKeySetIter implements Iterator<PyObject> {
			Iterator<Object> itr;
			Object cache;

			PyNativeRefHoldingStringMapKeySetIter(Iterator<Object> itr) {
				this.itr = itr;
			}

			public boolean hasNext() {
				return itr.hasNext();
			}

			public PyObject next() {
				cache = itr.next();
				return keyToPy(cache);
			}

			public void remove() {
				PyObject toRemove = backtable.get(cache);
				if (toRemove != null) nativeDecref(toRemove);
				itr.remove();
			}
		}

		@Override
		public Iterator<PyObject> iterator() {
			return new PyNativeRefHoldingStringMapKeySetIter(backend.iterator());
		}

		@Override
		public int size() {
			return backend.size();
		}
	}

	public Set<PyObject> pyKeySet() {
		return new PyNativeRefHoldingStringMapKeySetWrapper(getMap(), getMap().keySet());
	}

	//todo: Make several stuff below public in AbstractDict to simplify this implementation.
	static Object tojava(Object val) {
		//todo: Fix this for non-PyObjects (also in AbstractDict!)
		return val == null ? null : val instanceof PyObject ?
				((PyObject) val).__tojava__(Object.class) : val;
	}

	/** Basic implementation of Entry that just holds onto a key and value and returns them. */
	class SimpleEntry implements Entry {

		protected Object key;

		protected Object value;

		public SimpleEntry(Object key, Object value) {
			this.key = key;
			this.value = value;
		}

		public Object getKey() {
			return key;
		}

		public Object getValue() {
			return value;
		}

		@Override
		public boolean equals(Object o) {
			if (!(o instanceof Map.Entry)) {
				return false;
			}
			Map.Entry e = (Map.Entry)o;
			return eq(key, e.getKey()) && eq(value, e.getValue());
		}

		private boolean eq(Object o1, Object o2) {
			return o1 == null ? o2 == null : o1.equals(o2);
		}

		@Override
		public int hashCode() {
			return ((key == null) ? 0 : key.hashCode()) ^ ((value == null) ? 0 : value.hashCode());
		}

		@Override
		public String toString() {
			return key + "=" + value;
		}

		public Object setValue(Object val) {
			throw new UnsupportedOperationException("Not supported by this view");
		}
	}

	/**
	 * Wrapper for a Entry object returned from the java.util.Set object which in turn is returned by
	 * the entrySet method of java.util.Map. This is needed to correctly convert from PyObjects to java
	 * Objects. Note that we take care in the equals and hashCode methods to make sure these methods are
	 * consistent with Entry objects that contain java Objects for a value so that on the java side they
	 * can be reliable compared.
	 */
	class PyToJavaMapEntry extends SimpleEntry {

		/** Create a copy of the Entry with Py.None converted to null */
		PyToJavaMapEntry(Entry<Object,PyObject> entry) {
			super(entry.getKey(), entry.getValue());
		}

		@Override
		public boolean equals(Object o) {
			if (o == null || !(o instanceof Entry)) {
				return false;
			}
			Entry me = new JavaToPyMapEntry((Entry)o);
			return o.equals(me);
		}

		/* tojava is called in getKey and getValue so the raw key and value can be
		   used to create a new SimpleEntry in getEntry. */
		@Override
		public Object getKey() {
			return tojava(key);
		}

		@Override
		public Object getValue() {
			return tojava(value);
		}

		/**
		 * @return an entry that returns the original values given to this entry.
		 */
		public Entry getEntry() {
			return new SimpleEntry(key, value);
		}

		@Override
		public int hashCode() {
			return ((key == null) ? 0 : key.hashCode()) ^ ((value == null) ? 0 : value.hashCode());
		}

	}

	/**
	 * MapEntry Object for java MapEntry objects passed to the java.util.Set interface which is returned
	 * by the entrySet method of PyDictionary. Essentially like PyTojavaMapEntry, but going the other
	 * way converting java Objects to PyObjects.
	 */
	class JavaToPyMapEntry extends SimpleEntry {

		public JavaToPyMapEntry(Entry entry) {
			super(Py.java2py(entry.getKey()), Py.java2py(entry.getValue()));
		}
	}

	abstract class PyNativeRefHoldingMapSet extends AbstractSet {

		private final Set<Entry<Object,PyObject>> coll;

		PyNativeRefHoldingMapSet(Set<Entry<Object,PyObject>> coll) {
			this.coll = coll;
		}

		abstract Object toJava(Entry<Object,PyObject> obj);

		abstract Object toPython(Object obj);

		@Override
		public int size() {
			return coll.size();
		}

		@Override
		public boolean contains(Object o) {
			return coll.contains(toPython(o));
		}

		@Override
		 public boolean remove(Object o) {
			 return coll.remove(toPython(o));
		}

		@Override
		public void clear() {
			coll.clear();
		}

		/* Iterator wrapper class returned by the PyMapSet iterator
		   method. We need this wrapper to return PyToJavaMapEntry objects
		   for the 'next()' method. */
		class PySetIter implements Iterator {
			Iterator<Entry<Object,PyObject>> itr;
			Entry<Object,PyObject> cache;

			PySetIter(Iterator<Entry<Object,PyObject>> itr) {
				this.itr = itr;
			}

			public boolean hasNext() {
				return itr.hasNext();
			}

			public Object next() {
				cache = itr.next();
				return toJava(cache);
			}

			public void remove() {
				nativeDecref(cache.getValue());
				itr.remove();
			}
		}

		@Override
		public Iterator iterator() {
			return new PySetIter(coll.iterator());
		}
	}

	class PyNativeRefHoldingMapEntrySet extends PyNativeRefHoldingMapSet {

		PyNativeRefHoldingMapEntrySet(Set<Entry<Object,PyObject>> coll) {
			super(coll);
		}

		/* We know that PyMapEntrySet will only contains entries, so if the object being passed in is
		   null or not an Entry, then return null which will match nothing for remove and contains
		   methods. */
		@Override
		Object toPython(Object o) {
			if (o == null || !(o instanceof Entry)) {
				return null;
			}
			if (o instanceof PyToJavaMapEntry) {
				/* Use the original entry from PyDictionary */
				return ((PyToJavaMapEntry)o).getEntry();
			} else {
				return new JavaToPyMapEntry((Entry)o);
			}
		}

		@Override
		Object toJava(Entry<Object,PyObject> o) {
			return new PyToJavaMapEntry(o);
		}
	}

	/** @see java.util.Map#entrySet() */
	public Set entrySet() {
		return new PyNativeRefHoldingMapEntrySet(getMap().entrySet());
	}
}
