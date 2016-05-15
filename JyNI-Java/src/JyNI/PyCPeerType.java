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
import org.python.core.ThreadState;
import org.python.core.PyDictionary;
import org.python.core.PyString;
import org.python.core.PyTuple;
import org.python.core.PyType;
import org.python.core.PyObject;
import org.python.core.PyException;
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

//	public PyCPeerType(long objectHandle) {
//		super(fromClass(PyType.class));
//		System.out.println("created raw PyCPeerType");
//		this.objectHandle = objectHandle;
//		JyNI.CPeerHandles.put(objectHandle, this);
//	}

	public PyCPeerType(long objectHandle, String name, PyObject dict) {
		this(objectHandle, name, dict, fromClass(PyType.class));
//		System.out.println("handle_name_dict");
//		super(fromClass(PyType.class));
//		this.objectHandle = objectHandle;
//		super.name = name;
//		//super.setName(name);
//		if (dict != null) super.dict = dict;
//		JyNI.CPeerHandles.put(objectHandle, this);
	}

	public PyCPeerType(long objectHandle, String name, PyObject dict, PyType metatype) {
		super(metatype);
//		System.out.println("created PyCPeerType "+name+" withe metatype "+metatype.getName());
		this.objectHandle = objectHandle;
		super.name = name;
		//super.setName(name);
		if (dict != null) super.dict = dict;
		JyNI.CPeerHandles.put(objectHandle, this);
	}

//	public PyType getType() {
//		//System.out.println("PyCPeerType.getType");
//		//Todo: Support arbitrary metatypes.
//		return super.getType();
//	}

//	public boolean isSubType(PyType supertype) {
//		System.out.println("PyCPeerType.isSubType");
//		boolean result = super.isSubType(supertype);
//		System.out.println(getName()+" subtype of "+supertype.getName()+"? "+result);
//		return result;
//	}

	@Override
	public PyObject __call__(PyObject[] args, String[] keywords) {
//		System.out.println("CPeerType called: "+this);
//		if (this.toString().equals("<class 'ctypes.CFunctionType'>"))
//		{
//			System.out.println(args[0]);
//			System.out.println(args[0].getClass());
//		}
		//System.out.println("args: "+args);
//		System.out.println("arg count: "+args.length);
		//if (keywords !=)
		//System.out.println("PeerCall kw: "+keywords.length);
		/*for(int i = 0; i < keywords.length; ++i)
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

//	public void addMethod(PyBuiltinMethod meth) {
//		System.out.println("PyCPeerType.addMethod: "+meth);
//	}

//	int findAttrCount = 0;
//	WeakReference<PyObject> classCache = null;
	@Override
	public PyObject __findattr_ex__(String name) {
//		if (name.equals("__class__") && classCache != null) {
//			PyObject cc = classCache.get();
//			if (cc != null) return cc;
//			else classCache = null;
//		}
		//System.out.println("Look for attribute "+name+" in PyCPeerType "+this.name);//+" "+(findAttrCount++));
		ThreadState tstate = Py.getThreadState();
		long ts = JyTState.prepareNativeThreadState(tstate);
		PyObject er = JyNI.getAttrString(objectHandle, name, ts);
		if (er == null && tstate.exception != null && tstate.exception.type == Py.AttributeError) {
			// In attribute error case we give it another chance by redirecting to super.
			//PyException tmp = tstate.exception;
			// We clear exception to see whether super inserts its own exception.
			tstate.exception = null;
			// super call is now responsible to do exception stuff
			return super.__findattr_ex__(name);

//			er = super.__findattr_ex__(name);
//			if (er != null) {
//				// If result is promising we succeed, check for exception by super call though.
//				// (This check can probably be removed since super would have thrown exception
//				// if any and also should not insert exception and return non-null)
//				er = JyNI.maybeExc(er);
//				return er;
//			} else if (tstate.exception == null)
//				// If super behaves strange - returns null without throwing exception - we at least
//				// remember our original exception and restore it:
//				tstate.exception = tmp;
		}
		er = JyNI.maybeExc(er);
//		if (name.equals("__class__") && classCache == null) classCache = new WeakReference(er);
		return er != null ? er : super.__findattr_ex__(name);
		//return super.__findattr_ex__(name);
	}

	@Override
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

	@Override
	public PyString __str__() {
		PyString er = (PyString) JyNI.maybeExc(JyNI.PyObjectAsPyString(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		return er == null ? (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState()))) : er;
	}

	@Override
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
	@Override
	public PyObject __finditem__(PyObject key) {
		return JyNI.maybeExc(JyNI.getItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	@Override
	public void __setitem__(PyObject key, PyObject value) {
		JyNI.maybeExc(JyNI.setItem(objectHandle, key, value,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	@Override
	public void __delitem__(PyObject key) {
		JyNI.maybeExc(JyNI.delItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}

	@Override
	public int __len__() {
		int er = JyNI.PyObjectLength(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc();
		return er;
	}

// It appears number operators work via descriptors on native side, because
// Jython would not treat PyCPeer class family as builtins. So corresponding
// magic methods for numbers need not be implemented here. We will add this
// implementation, if a good reason for this is discovered.

//	@Override
//	public PyObject __mul__(PyObject other) {
//        return JyNI.maybeExc(JyNI.JyNI_PyNumber_Multiply(objectHandle, other,
//        		JyTState.prepareNativeThreadState(Py.getThreadState())));
//    }
//
//	@Override
//	public PyObject __rmul__(PyObject other) {
//        return JyNI.maybeExc(JyNI.JyNI_PyNumber_Multiply(objectHandle, other,
//        		JyTState.prepareNativeThreadState(Py.getThreadState())));
//    }

	@Override
	public String toString() {
		return JyNI.PyObjectAsString(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState()));
	}

	@Override
	public long getHandle() {
		return objectHandle;
	}

//	@Override
//	public PyTuple getMro() {
//		System.out.println("getMro of "+name+"/"+objectHandle+": "+(mro == null ? mro : new PyTuple(mro)));
//		return super.getMro();
//        //return mro == null ? Py.EmptyTuple : new PyTuple(mro);
//    }

	@Override
	public void __del_builtin__() {
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, refHandle);
	}

	/*
	 * Though it is discouraged, we use finalize to tidy up the
	 * native references of this peer. We might replace this by
	 * a better solution in the future.
	 * (This will probably be as follows:
	 * Track all peers in a Set using WeakReferences
	 * and have these references registered in a ReferenceQueue.
	 * From Time to time poll things from the queue and tidy
	 * up or have a thread permanently waiting on the queue.)
	 */
//	protected void finalize() throws Throwable {
//		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, refHandle);
//	}
}
