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
import org.python.core.finalization.FinalizableBuiltin;

public class PyCPeerType extends PyType implements CPeerInterface, FinalizableBuiltin {

	public static final long NB_ADD                  = 0x1l;
	public static final long NB_SUBTRACT             = 0x2l;
	public static final long NB_MULTIPLY             = 0x4l;
	public static final long NB_DIVIDE               = 0x8l;
	public static final long NB_REMAINDER            = 0x10l;
	public static final long NB_DIVMOD               = 0x20l;
	public static final long NB_POWER                = 0x40l;
	public static final long NB_NEGATIVE             = 0x80l;
	public static final long NB_POSITIVE             = 0x100l;
	public static final long NB_ABSOLUTE             = 0x200l;
	public static final long NB_NONZERO              = 0x400l;
	public static final long NB_INVERT               = 0x800l;
	public static final long NB_LSHIFT               = 0x1000l;
	public static final long NB_RSHIFT               = 0x2000l;
	public static final long NB_AND                  = 0x4000l;
	public static final long NB_XOR                  = 0x8000l;
	public static final long NB_OR                   = 0x10000l;
	public static final long NB_COERCE               = 0x20000l;
	public static final long NB_INT                  = 0x40000l;
	public static final long NB_LONG                 = 0x80000l;
	public static final long NB_FLOAT                = 0x100000l;
	public static final long NB_OCT                  = 0x200000l;
	public static final long NB_HEX                  = 0x400000l;
	public static final long NB_INPLACE_ADD          = 0x800000l;
	public static final long NB_INPLACE_SUBTRACT     = 0x1000000l;
	public static final long NB_INPLACE_MULTIPLY     = 0x2000000l;
	public static final long NB_INPLACE_DIVIDE       = 0x4000000l;
	public static final long NB_INPLACE_REMAINDER    = 0x8000000l;
	public static final long NB_INPLACE_POWER        = 0x10000000l;
	public static final long NB_INPLACE_LSHIFT       = 0x20000000l;
	public static final long NB_INPLACE_RSHIFT       = 0x40000000l;
	public static final long NB_INPLACE_AND          = 0x80000000l;
	public static final long NB_INPLACE_XOR          = 0x100000000l;
	public static final long NB_INPLACE_OR           = 0x200000000l;
	public static final long NB_FLOOR_DIVIDE         = 0x400000000l;
	public static final long NB_TRUE_DIVIDE          = 0x800000000l;
	public static final long NB_INPLACE_FLOOR_DIVIDE = 0x1000000000l;
	public static final long NB_INPLACE_TRUE_DIVIDE  = 0x2000000000l;
	public static final long NB_INDEX                = 0x4000000000l;
	public static final long SQ_LENGTH               = 0x8000000000l;
	public static final long SQ_CONCAT               = 0x10000000000l;
	public static final long SQ_REPEAT               = 0x20000000000l;
	public static final long SQ_ITEM                 = 0x40000000000l;
	public static final long SQ_SLICE                = 0x80000000000l;
	public static final long SQ_ASS_ITEM             = 0x100000000000l;
	public static final long SQ_ASS_SLICE            = 0x200000000000l;
	public static final long SQ_CONTAINS             = 0x400000000000l;
	public static final long SQ_INPLACE_CONCAT       = 0x800000000000l;
	public static final long SQ_INPLACE_REPEAT       = 0x1000000000000l;
	public static final long MP_LENGTH               = 0x2000000000000l;
	public static final long MP_SUBSCRIPT            = 0x4000000000000l;
	public static final long MP_ASS_SUBSCRIPT        = 0x8000000000000l;
	public static final long BF_GETREADBUFFER        = 0x10000000000000l;
	public static final long BF_GETWRITEBUFFER       = 0x20000000000000l;
	public static final long BF_GETSEGCOUNT          = 0x40000000000000l;
	public static final long BF_GETCHARBUFFER        = 0x80000000000000l;
	public static final long BF_GETBUFFER            = 0x100000000000000l;
	public static final long BF_RELEASEBUFFER        = 0x200000000000000l;

	public long nativeMethodFlags = 0;

	public static void printNativeMethodFlags(long flags) {
		if ((flags & NB_ADD) != 0) System.out.println("NB_ADD");
		if ((flags & NB_SUBTRACT) != 0) System.out.println("NB_SUBTRACT");
		if ((flags & NB_MULTIPLY) != 0) System.out.println("NB_MULTIPLY");
		if ((flags & NB_DIVIDE) != 0) System.out.println("NB_DIVIDE");
		if ((flags & NB_REMAINDER) != 0) System.out.println("NB_REMAINDER");
		if ((flags & NB_DIVMOD) != 0) System.out.println("NB_DIVMOD");
		if ((flags & NB_POWER) != 0) System.out.println("NB_POWER");
		if ((flags & NB_NEGATIVE) != 0) System.out.println("NB_NEGATIVE");
		if ((flags & NB_POSITIVE) != 0) System.out.println("NB_POSITIVE");
		if ((flags & NB_ABSOLUTE) != 0) System.out.println("NB_ABSOLUTE");
		if ((flags & NB_NONZERO) != 0) System.out.println("NB_NONZERO");
		if ((flags & NB_INVERT) != 0) System.out.println("NB_INVERT");
		if ((flags & NB_LSHIFT) != 0) System.out.println("NB_LSHIFT");
		if ((flags & NB_RSHIFT) != 0) System.out.println("NB_RSHIFT");
		if ((flags & NB_AND) != 0) System.out.println("NB_AND");
		if ((flags & NB_XOR) != 0) System.out.println("NB_XOR");
		if ((flags & NB_OR) != 0) System.out.println("NB_OR");
		if ((flags & NB_COERCE) != 0) System.out.println("NB_COERCE");
		if ((flags & NB_INT) != 0) System.out.println("NB_INT");
		if ((flags & NB_LONG) != 0) System.out.println("NB_LONG");
		if ((flags & NB_FLOAT) != 0) System.out.println("NB_FLOAT");
		if ((flags & NB_OCT) != 0) System.out.println("NB_OCT");
		if ((flags & NB_HEX) != 0) System.out.println("NB_HEX");
		if ((flags & NB_INPLACE_ADD) != 0) System.out.println("NB_INPLACE_ADD");
		if ((flags & NB_INPLACE_SUBTRACT) != 0) System.out.println("NB_INPLACE_SUBTRACT");
		if ((flags & NB_INPLACE_MULTIPLY) != 0) System.out.println("NB_INPLACE_MULTIPLY");
		if ((flags & NB_INPLACE_DIVIDE) != 0) System.out.println("NB_INPLACE_DIVIDE");
		if ((flags & NB_INPLACE_REMAINDER) != 0) System.out.println("NB_INPLACE_REMAINDER");
		if ((flags & NB_INPLACE_POWER) != 0) System.out.println("NB_INPLACE_POWER");
		if ((flags & NB_INPLACE_LSHIFT) != 0) System.out.println("NB_INPLACE_LSHIFT");
		if ((flags & NB_INPLACE_RSHIFT) != 0) System.out.println("NB_INPLACE_RSHIFT");
		if ((flags & NB_INPLACE_AND) != 0) System.out.println("NB_INPLACE_AND");
		if ((flags & NB_INPLACE_XOR) != 0) System.out.println("NB_INPLACE_XOR");
		if ((flags & NB_INPLACE_OR) != 0) System.out.println("NB_INPLACE_OR");
		if ((flags & NB_FLOOR_DIVIDE) != 0) System.out.println("NB_FLOOR_DIVIDE");
		if ((flags & NB_TRUE_DIVIDE) != 0) System.out.println("NB_TRUE_DIVIDE");
		if ((flags & NB_INPLACE_FLOOR_DIVIDE) != 0) System.out.println("NB_INPLACE_FLOOR_DIVIDE");
		if ((flags & NB_INPLACE_TRUE_DIVIDE) != 0) System.out.println("NB_INPLACE_TRUE_DIVIDE");
		if ((flags & NB_INDEX) != 0) System.out.println("NB_INDEX");
		if ((flags & SQ_LENGTH) != 0) System.out.println("SQ_LENGTH");
		if ((flags & SQ_CONCAT) != 0) System.out.println("SQ_CONCAT");
		if ((flags & SQ_REPEAT) != 0) System.out.println("SQ_REPEAT");
		if ((flags & SQ_ITEM) != 0) System.out.println("SQ_ITEM");
		if ((flags & SQ_SLICE) != 0) System.out.println("SQ_SLICE");
		if ((flags & SQ_ASS_ITEM) != 0) System.out.println("SQ_ASS_ITEM");
		if ((flags & SQ_ASS_SLICE) != 0) System.out.println("SQ_ASS_SLICE");
		if ((flags & SQ_CONTAINS) != 0) System.out.println("SQ_CONTAINS");
		if ((flags & SQ_INPLACE_CONCAT) != 0) System.out.println("SQ_INPLACE_CONCAT");
		if ((flags & SQ_INPLACE_REPEAT) != 0) System.out.println("SQ_INPLACE_REPEAT");
		if ((flags & MP_LENGTH) != 0) System.out.println("MP_LENGTH");
		if ((flags & MP_SUBSCRIPT) != 0) System.out.println("MP_SUBSCRIPT");
		if ((flags & MP_ASS_SUBSCRIPT) != 0) System.out.println("MP_ASS_SUBSCRIPT");
		if ((flags & BF_GETREADBUFFER) != 0) System.out.println("BF_GETREADBUFFER");
		if ((flags & BF_GETWRITEBUFFER) != 0) System.out.println("BF_GETWRITEBUFFER");
		if ((flags & BF_GETSEGCOUNT) != 0) System.out.println("BF_GETSEGCOUNT");
		if ((flags & BF_GETCHARBUFFER) != 0) System.out.println("BF_GETCHARBUFFER");
		if ((flags & BF_GETBUFFER) != 0) System.out.println("BF_GETBUFFER");
		if ((flags & BF_RELEASEBUFFER) != 0) System.out.println("BF_RELEASEBUFFER");
	}
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

	public PyCPeerType(long objectHandle, String name, PyObject dict, long natMethFlags) {
		this(objectHandle, name, dict, natMethFlags, fromClass(PyType.class));
	}

	public PyCPeerType(long objectHandle, String name, PyObject dict, long natMethFlags, PyType metatype) {
		super(metatype);
//		System.out.println("created PyCPeerType "+name+" withe metatype "+metatype.getName());
		this.objectHandle = objectHandle;
		super.name = name;
		this.nativeMethodFlags = natMethFlags;
		//super.setName(name);
		if (dict != null) super.dict = dict;
		JyNI.CPeerHandles.put(objectHandle, this);
//		System.out.println("created PyCPeerType "+name);
//		if (nativeMethodFlags == 0) System.out.println("no flags set");
//		else printNativeMethodFlags(nativeMethodFlags);
	}

//	public boolean isSubType(PyType supertype) {
//		System.out.println("PyCPeerType.isSubType");
//		boolean result = super.isSubType(supertype);
//		System.out.println(getName()+" subtype of "+supertype.getName()+"? "+result);
//		return result;
//	}

	@Override
	public PyObject __call__(PyObject[] args, String[] keywords) {
		PyObject result;
		if (keywords.length == 0)
			result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle,
				args.length == 0 ? Py.EmptyTuple : new PyTuple(args, false), null,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
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
	}

//	public void addMethod(PyBuiltinMethod meth) {
//		System.out.println("PyCPeerType.addMethod: "+meth);
//	}

	@Override
	public PyObject __findattr_ex__(String name) {
		//System.out.println("Look for attribute "+name+" in PyCPeerType "+this.name);
		ThreadState tstate = Py.getThreadState();
		long ts = JyTState.prepareNativeThreadState(tstate);
		PyObject er = JyNI.getAttrString(objectHandle, name, ts);
		if (er == null && tstate.exception != null && tstate.exception.type == Py.AttributeError) {
			// In attribute error case we give it another chance by redirecting to super.
			//PyException tmp = tstate.exception;
			// We clear exception to see whether super inserts its own exception.
			tstate.exception = null;
			JyNI.JyNI_exc = null;
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
}
