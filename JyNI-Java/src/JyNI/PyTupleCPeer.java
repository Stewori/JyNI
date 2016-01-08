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

import java.util.HashMap;

import org.python.core.Py;
import org.python.core.PyDictionary;
import org.python.core.PyObject;
import org.python.core.PyString;
import org.python.core.PyTuple;
import org.python.core.finalization.FinalizableBuiltin;

import JyNI.gc.DefaultTraversableGCHead;
import JyNI.gc.JyGCHead;
import JyNI.gc.JyVisitproc;
import JyNI.gc.TraversableGCHead;

/**
 * Specialized CPeer classes are used to wrap native custom types that extend
 * builtin types. For consistency the Java-counterpart must also be something
 * that extends the Java-counterpart of the builtin. For now we don't support
 * nesting of this principle.
 *
 * @author Stefan Richthofer
 */
public class PyTupleCPeer extends PyTuple implements
		CPeerNativeDelegateSubtype, FinalizableBuiltin, TraversableGCHead {

	public long objectHandle;
	protected Object links;

	/**
	 * This constructor signature is obligatory for every specialized
	 * CPeer-class.
	 */
	public PyTupleCPeer(long objectHandle, PyCPeerType subtype, PyObject[] init) {
		super(subtype, init);
		this.objectHandle = objectHandle;
		JyNI.CPeerHandles.put(objectHandle, this);
	}

	@Override
	public PyObject __call__(PyObject[] args, String[] keywords) {
		PyObject result;
		if (keywords.length == 0)
			result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle,
				args.length == 0 ? Py.EmptyTuple : new PyTuple(args, false), null,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		else {
			HashMap<PyObject, PyObject> back = new HashMap<PyObject, PyObject>(keywords.length);
			for (int i = 0; i < keywords.length; ++i) {
				back.put(Py.newString(keywords[i]), args[args.length-keywords.length+i]);
			}
			
			if (args.length > keywords.length) {
				PyObject[] args2 = new PyObject[args.length - keywords.length];
				System.arraycopy(args, 0, args2, 0, args2.length);
				result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle, new PyTuple(args2, false),
					new PyDictionary(back), JyTState.prepareNativeThreadState(Py.getThreadState())));
			} else
				result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle, Py.EmptyTuple, new PyDictionary(back),
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		}
		return result == null ? super.__call__(args, keywords) : result;
	}

	@Override
	public PyObject __findattr_ex__(String name) {
		//System.out.println("Look for attribute "+name+" in PyCPeerType "+this.name+" "+(findAttrCount++));
		long ts = JyTState.prepareNativeThreadState(Py.getThreadState());
		PyObject result = JyNI.maybeExc(JyNI.getAttrString(objectHandle, name, ts));
		return result != null ? result : super.__findattr_ex__(name);
	}

	@Override
	public PyString __str__() {
		PyString result = (PyString) JyNI.maybeExc(JyNI.PyObjectAsPyString(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		if (result == null) result = (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		return result == null ? super.__str__() : result;
	}

	@Override
	public PyString __repr__() {
		PyString result = (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		return result == null ? super.__repr__() : result;
	}

/* The naive implementation of find-item methods below causes infinite looping
 * between native dict and Jython dict, e.g. in StgDict-case.
 * Todo: Invent something to prevent this. Since ctypes appears to work okay
 * with the code below uncommented, we can maybe shift this fix to alpha-4.
 * So far, StgDict etc cannot be cleanly accessed from Java-side.
 * We somehow must distinguish native calls from Jython calls here.
 * This implies we must change method signatures somehow, e.g. introduce
 * special variants for native call-ins.
 * This implies, native side must perform a case distinction, whether Jython
 * dict methods are called from within a subtype or from somewhere else (doesn't it?).
 */
	@Override
	public PyObject __finditem__(PyObject key) {
//		return super.__finditem__(key);
//		System.out.println("__finditem__: "+key);
//		System.out.println(getType().getName());
		PyObject result = JyNI.maybeExc(JyNI.getItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
//		System.out.println("done native __finditem__");
		return result != null ? result : super.__finditem__(key);
	}

	@Override
	public void __setitem__(PyObject key, PyObject value) {
//		System.out.println("__setitem__: "+key);
//		super.__setitem__(key, value);
		int er = JyNI.setItem(objectHandle, key, value,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc(er);
		if (er != 0) super.__setitem__(key, value);
	}

	@Override
	public void __delitem__(PyObject key) {
//		System.out.println("__delitem__");
//		super.__delitem__(key);
		int er = JyNI.delItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc(er);
		if (er != 0) super.__delitem__(key);
	}

	@Override
	public int __len__() {
//		System.out.println("__len__");
//		return super.__len__();
		int er = JyNI.PyObjectLength(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc();
		if (er == JyNI.NATIVE_INT_METHOD_NOT_IMPLEMENTED) return super.__len__();
		else return er;
	}

//	@Override
//	public String toString() {
//		return JyNI.PyObjectAsString(objectHandle,
//			JyTState.prepareNativeThreadState(Py.getThreadState()));
//	}

	@Override
	public long getHandle() {
		return objectHandle;
	}

	@Override
	public void __del_builtin__() {
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, 0);
	}

	@Override
	public PyObject super__call__(PyObject[] args, String[] keywords) {return super.__call__(args, keywords);}
	@Override
	public PyObject super__findattr_ex__(String name) {return super.__findattr_ex__(name);}
	@Override
	public void super__setattr__(String name, PyObject value) {super.__setattr__(name, value);}
	@Override
	public PyString super__str__() {return super.__str__();}
	@Override
	public PyString super__repr__() {return super.__repr__();}
	@Override
	public PyObject super__finditem__(PyObject key) {return super.__finditem__(key);}
	@Override
	public void super__setitem__(PyObject key, PyObject value) {super.__setitem__(key, value);}
	@Override
	public void super__delitem__(PyObject key) {super.__delitem__(key);}
	@Override
	public int super__len__() {return super.__len__();}
	@Override
	public String super_toString() {return super.toString();}


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
