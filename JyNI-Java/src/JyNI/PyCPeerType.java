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

import java.util.HashMap;

import org.python.core.Py;
import org.python.core.PyDictionary;
import org.python.core.PyString;
import org.python.core.PyTuple;
import org.python.core.PyType;
import org.python.core.PyObject;

public class PyCPeerType extends PyType {
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
		//System.out.println("CPeerType called...");
		//System.out.println("args: "+args);
		//System.out.println("arg count: "+args.length);
		//if (keywords !=)
		/*System.out.println("PeerCall kw: "+keywords.length);
		for(int i = 0; i < keywords.length; ++i)
			System.out.println(keywords[i]);
		System.out.println("PeerCall args: "+args.length);
		for(int i = 0; i < args.length; ++i)
			System.out.println(args[i]);*/
		//PyObject er =
		if (keywords.length == 0)
			return JyNI.callPyCPeer(objectHandle,
				args.length == 0 ? Py.EmptyTuple : new PyTuple(args, false), null,
				JyTState.prepareNativeThreadState(Py.getThreadState()));//Py.None);
		else {
			//todo: Use PyStringMap here... much work needs to be done to make the peer dictobject accept this
			HashMap<PyObject, PyObject> back = new HashMap<PyObject, PyObject>(keywords.length);
			for (int i = 0; i < keywords.length; ++i) {
				back.put(Py.newString(keywords[i]), args[args.length-keywords.length+i]);
			}
			
			if (args.length > keywords.length) {
				PyObject[] args2 = new PyObject[args.length - keywords.length];
				System.arraycopy(args, 0, args2, 0, args2.length);
				return JyNI.callPyCPeer(objectHandle, new PyTuple(args2, false),
					new PyDictionary(back), JyTState.prepareNativeThreadState(Py.getThreadState()));
			} else
				return JyNI.callPyCPeer(objectHandle, Py.EmptyTuple, new PyDictionary(back),
					JyTState.prepareNativeThreadState(Py.getThreadState()));
		}
		
		/*System.out.println("Call er:");
		System.out.println(er);
		System.out.println(er.getClass().getName());
		System.out.println(er.getType().getName());*/
		//return er;
	}

	public PyObject __findattr_ex__(String name) {
		//System.out.println("Look for attribute "+name+" in PyCPeerType");
		PyObject er = JyNI.getAttrString(objectHandle, name,
			JyTState.prepareNativeThreadState(Py.getThreadState()));
		return er != null ? er : Py.None;
		//return super.__findattr_ex__(name);
	}

	public PyString __repr__() {
//		System.out.println("PyCPeerType__repr__");
//		System.out.println(name);
		return (PyString) JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState()));
	}

	public String toString() {
		return JyNI.PyObjectAsString(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState()));
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
	protected void finalize() throws Throwable {
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, refHandle);
	}
}
