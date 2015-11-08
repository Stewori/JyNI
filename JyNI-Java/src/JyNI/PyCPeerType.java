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


package JyNI;

import java.util.HashMap;
//import java.lang.ref.WeakReference;

import org.python.core.Py;
import org.python.core.PyDictionary;
import org.python.core.PyString;
import org.python.core.PyTuple;
import org.python.core.PyType;
import org.python.core.PyObject;
import org.python.core.finalization.FinalizableBuiltin;

public class PyCPeerType extends PyType implements CPeerInterface, FinalizableBuiltin {
	/*
	public static long tp_name; // For printing, in format "<module>.<name>"
	public static long tp_basicsize, tp_itemsize; // For allocation

	// Methods to implement standard operations

	public static long tp_dealloc;
	public static long tp_print;
	public static long tp_getattr;
	public static long tp_setattr;
	public static long tp_compare;
	public static long tp_repr;

	// Method suites for standard classes

	public static long tp_as_number;
	public static long tp_as_sequence;
	public static long tp_as_mapping;

	// More standard operations (here for binary compatibility)

	public static long tp_hash;
	public static long tp_call;
	public static long tp_str;
	public static long tp_getattro;
	public static long tp_setattro;

	// Functions to access object as input/output buffer
	public static long tp_as_buffer;

	// Flags to define presence of optional/expanded features
	public static long tp_flags;

	public static long tp_doc; // Documentation string

	// Assigned meaning in release 2.0
	// call function for all accessible objects
	public static long tp_traverse;

	// delete references to contained objects
	public static long tp_clear;

	// Assigned meaning in release 2.1
	// rich comparisons
	public static long tp_richcompare;

	// weak reference enabler
	public static long tp_weaklistoffset;

	// Added in release 2.2
	// Iterators
	public static long tp_iter;
	public static long tp_iternext;

	// Attribute descriptor and subclassing stuff
	public static long tp_methods;
	public static long tp_members;
	public static long tp_getset;
	public static long tp_base;
	public static long tp_dict;
	public static long tp_descr_get;
	public static long tp_descr_set;
	public static long tp_dictoffset;
	public static long tp_init;
	public static long tp_alloc;
	public static long tp_new;
	public static long tp_free; // Low-level free-memory routine
	public static long tp_is_gc; // For PyObject_IS_GC
	public static long tp_bases;
	public static long tp_mro; // method resolution order
	public static long tp_cache;
	public static long tp_subclasses;
	public static long tp_weaklist;
	public static long tp_del;

	// Type attribute cache version tag. Added in version 2.6
	public static long tp_version_tag;*/

	public long objectHandle, refHandle;

	public PyCPeerType(long objectHandle) {
		super(fromClass(PyType.class));
		this.objectHandle = objectHandle;
		JyNI.CPeerHandles.put(objectHandle, this);
	}

	public PyCPeerType(long objectHandle, String name, PyObject dict) {
		super(fromClass(PyType.class));
		this.objectHandle = objectHandle;
		super.name = name;
		//super.setName(name);
		if (dict != null) super.dict = dict;
		JyNI.CPeerHandles.put(objectHandle, this);
	}

//	public String getName()
//	{
//		String er = super.getName();
//		//return "PyCPeer";
//		System.out.println("PyCPeerTypeGetName: "+er);
//		return er;
//	}

	public PyObject __call__(PyObject[] args, String[] keywords) {
		//System.out.println("CPeerType called: "+this);
		//System.out.println("args: "+args);
		//System.out.println("arg count: "+args.length);
		//if (keywords !=)
		/*System.out.println("PeerCall kw: "+keywords.length);
		for(int i = 0; i < keywords.length; ++i)
			System.out.println(keywords[i]);
		System.out.println("PeerCall args: "+args.length);
		for(int i = 0; i < args.length; ++i)
			System.out.println(args[i]);*/
		PyObject result;
		if (keywords.length == 0)
			result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle,
				args.length == 0 ? Py.EmptyTuple : new PyTuple(args, false), null,
				JyTState.prepareNativeThreadState(Py.getThreadState())));//Py.None);
		else {
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
				result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle, Py.EmptyTuple, new PyDictionary(back),
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		}
		if (result == null)
			throw Py.TypeError(String.format("'%s' object is not callable", getType().fastGetName()));
		else return result;
		/*System.out.println("Call er:");
		System.out.println(er);
		System.out.println(er.getClass().getName());
		System.out.println(er.getType().getName());*/
		//return er;
	}

//	int findAttrCount = 0;
//	WeakReference<PyObject> classCache = null;
	public PyObject __findattr_ex__(String name) {
//		if (name.equals("__class__") && classCache != null) {
//			PyObject cc = classCache.get();
//			if (cc != null) return cc;
//			else classCache = null;
//		}
//		System.out.println("Look for attribute "+name+" in PyCPeerType "+this.name+" "+(findAttrCount++));
		long ts = JyTState.prepareNativeThreadState(Py.getThreadState());
		PyObject er = JyNI.getAttrString(objectHandle, name, ts);
//		System.out.println("Result: "+er);
		er = JyNI.maybeExc(er);
//		if (name.equals("__class__") && classCache == null) classCache = new WeakReference(er);
		return er != null ? er : Py.None;
		//return super.__findattr_ex__(name);
	}

	public void __setattr__(String name, PyObject value) {
		/*
		 * Should we also try to call super?
		 * Actually we should assume that for native types the whole
		 * logic happens natively and if that fails somehow, we cannot
		 * fix that here. So probably it's best to just throw a natively
		 * occured exception if present.
		 */
		JyNI.maybeExc(JyNI.setAttrString(objectHandle, name, value,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		/*
		 * Should we attempt to call postSetattr(name)?
		 */
	}

	public PyString __str__() {
		PyString er = (PyString) JyNI.maybeExc(JyNI.PyObjectAsPyString(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		return er == null ? (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState()))) : er;
	}

	public PyString __repr__() {
//		System.out.println("PyCPeerType__repr__");
//		System.out.println(name);
		return (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	/* Yes, item-methods are not exactly a typical use-case for type-objects.
	 * However we still implement this forwarding here, because you can
	 * never know what crazy stuff native types come up with.
	 */
	public PyObject __finditem__(PyObject key) {
		return JyNI.maybeExc(JyNI.getItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	public void __setitem__(PyObject key, PyObject value) {
		JyNI.maybeExc(JyNI.setItem(objectHandle, key, value,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	public void __delitem__(PyObject key) {
		JyNI.maybeExc(JyNI.delItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	public int __len__() {
		int er = JyNI.PyObjectLength(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc();
		return er;
	}

	public String toString() {
		return JyNI.PyObjectAsString(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState()));
	}

	@Override
	public long getHandle() {
		return objectHandle;
	}

	/**
	 * Though it is discouraged, we use finalize to tidy up the
	 * native references of this peer. We might replace this by
	 * a better solution in the future.
	 * (This will probably be as follows:
	 * Track all peers in a Set using WeakReferences
	 * and have these references registered in a ReferenceQueue.
	 * From Time to time poll things from the queue and tidy
	 * up or have a thread permanently waiting on the queue.)
	 */
	@Override
	public void __del_builtin__() {
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, refHandle);
	}

//	protected void finalize() throws Throwable {
//		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, refHandle);
//	}
}
