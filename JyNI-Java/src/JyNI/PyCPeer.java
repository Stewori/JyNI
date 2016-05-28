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

import org.python.core.PyType;
import org.python.core.PyObject;
import org.python.core.PyString;
import org.python.core.PyTuple;
import org.python.core.PyDictionary;
import org.python.core.Py;
import org.python.core.finalization.FinalizableBuiltin;
import org.python.core.Untraversable;
import java.util.HashMap;

@Untraversable
public class PyCPeer extends PyObject implements CPeerInterface, FinalizableBuiltin {

	public long objectHandle;//, refHandle;

	public PyCPeer(long objectHandle, PyType subtype) {
		super(subtype);
//		if (subtype == null) System.out.println("PyCPeer with null-type");
//		else {
//			System.out.println("PyCPeer "+objectHandle+" of type:");
//			System.out.println(subtype.getName());
//		}
		this.objectHandle = objectHandle;
		//JyNI.CPeerHandles.put(objectHandle, this);
	}

	@Override
	public PyObject __call__(PyObject[] args, String[] keywords) {
		PyObject result = null;
		if (keywords.length == 0) {
			result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle,
					args.length == 0 ? Py.EmptyTuple : new PyTuple(args, false), null,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		} else {
			//todo: Use PyStringMap here... much work needs to be done to make the peer dictobject accept this
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
				result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle, Py.EmptyTuple,
					new PyDictionary(back), JyTState.prepareNativeThreadState(Py.getThreadState())));
		}
		if (result == null)
			// This is equivalent to
			// throw Py.TypeError(String.format("'%s' object is not callable", getType().fastGetName()));
			// since parent is PyObject. Using super the error message automatically reflects updates
			// to Jython-code.
			return super.__call__(args, keywords);
		else
			return result;
	}

	@Override
	public PyObject __findattr_ex__(String name) {
//		System.out.println("Look for attribute "+name+" in PyCPeer");
		PyObject er = JyNI.maybeExc(JyNI.getAttrString(objectHandle, name,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
//		System.out.println("result: "+er);
		return er != null ? er : Py.None;
		//return super.__findattr_ex__(name);
	}

	@Override
	public void __setattr__(String name, PyObject value) {
		JyNI.maybeExc(JyNI.setAttrString(objectHandle, name, value,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
    }

	@Override
	public PyObject __finditem__(PyObject key) {
		return JyNI.maybeExc(JyNI.getItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
//		throw Py.TypeError(String.format("'%.200s' object is unsubscriptable",
//				getType().fastGetName()));
	}

	@Override
	public void __setitem__(PyObject key, PyObject value) {
		JyNI.maybeExc(JyNI.setItem(objectHandle, key, value,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
//		throw Py.TypeError(String.format("'%.200s' object does not support item assignment",
//				getType().fastGetName()));
	}

	@Override
	public void __delitem__(PyObject key) {
		JyNI.maybeExc(JyNI.delItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
//		throw Py.TypeError(String.format("'%.200s' object doesn't support item deletion",
//                                         getType().fastGetName()));
	}

	@Override
	public int __len__() {
		int er = JyNI.PyObjectLength(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc();
		return er;
//		throw Py.TypeError(String.format("object of type '%.200s' has no len()",
//                                         getType().fastGetName()));
	}

	@Override
	public PyObject __get__(PyObject obj, PyObject type) {
//		System.out.println("PyCPeer.__get__ "+obj+"  "+type);
		PyObject result = JyNI.maybeExc(JyNI.descr_get(objectHandle, obj, type,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		return result != null ? result : super.__get__(obj, type);
	}

	@Override
	public void __set__(PyObject obj, PyObject value) {
		int result = JyNI.descr_set(objectHandle, obj, value,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc();
		if (result == JyNI.NATIVE_INT_METHOD_NOT_IMPLEMENTED)
			super.__set__(obj, value);
	}

	@Override
	public PyString __str__() {
		//System.out.println("PyCPeer__str__");
		//Object er = JyNI.PyObjectAsPyString(objectHandle);
		//return (PyString) JyNI.repr(objectHandle);
		PyString er = (PyString) JyNI.maybeExc(JyNI.PyObjectAsPyString(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		return er == null ? (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState()))) : er;
	}

	@Override
	public PyString __repr__() {
		//System.out.println("PyCPeer__repr__");
		return (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	@Override
	public String toString() {
		return JyNI.PyObjectAsString(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
	}

	/**
	 * Though it is discouraged in JNI-documentation,
	 * we use finalize to tidy up the native references
	 * of this peer. We plan to replace this by
	 * a better solution in the future.
	 * (This will probably be as follows:
	 * Track all peers in a Set using WeakReferences
	 * and have these references registered in a ReferenceQueue.
	 * From Time to time poll things from the queue and tidy
	 * up or have a thread permanently waiting on the queue.)
	 */
	@Override
	public void __del_builtin__() {
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, 0);
	}

	@Override
	public long getHandle() {
		return objectHandle;
	}
//	protected void finalize() throws Throwable {
//		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, 0);
//	}
}
