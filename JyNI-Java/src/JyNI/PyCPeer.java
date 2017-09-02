/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
 * 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
 * Python Software Foundation.
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
import org.python.core.PyException;
import org.python.core.PyFloat;
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
		PyObject res = JyNI.getItem(objectHandle, key,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_ITER) != 0)
		{
			try {
				JyNI.maybeExc(res);
			} catch (PyException exc) {
				if (exc.match(Py.IndexError)) {
					/* We compensate an incompatibility between behaviors of
					 * Jython's and CPythons PySequenceIter objects.
					 * While CPython's implementation aborts sanely on
					 * StopIteration or IndexError, Jython's implementation
					 * strictly requires StopIteration. So we replace IndexError
					 * by StopIteration, if we're an iterable.
					 */
					throw new PyException(Py.StopIteration, exc.value);
				} else throw exc;
			}
			return res;
		} else return JyNI.maybeExc(res);
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
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.MP_LENGTH) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyMapping_Length(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_LENGTH) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PySequence_Length(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		int er = JyNI.PyObjectLength(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState()));
		JyNI.maybeExc();
		return er;
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
//		System.out.println("PyCPeer__str__ "+getType().getName());
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

	@Override
	public void __del_builtin__() {
		//System.out.println(getClass().getSimpleName()+" finalize: "+objectHandle);
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, 0);
	}

	@Override
	public long getHandle() {
		return objectHandle;
	}

	/**
	 * Allow fast-succeed based on flags.
	 */
	@Override
	public boolean isNumberType() {
		long flags = ((PyCPeerType) objtype).nativeMethodFlags;
		if ((flags & PyCPeerType.NB_INT) != 0 || (flags & PyCPeerType.NB_FLOAT) != 0)
			return true;
		else return super.isNumberType();
	}

	/**
	 * Allow fast-succeed based on flags.
	 */
	@Override
	public boolean isMappingType() {
		long flags = ((PyCPeerType) objtype).nativeMethodFlags;
		if ((flags & PyCPeerType.MP_SUBSCRIPT) != 0 || (flags & PyCPeerType.MP_ASS_SUBSCRIPT) != 0)
			return true;
		else return super.isMappingType();
	}

	/**
	 * Allow fast-succeed based on flags.
	 */
	@Override
	public boolean isSequenceType() {
		long flags = ((PyCPeerType) objtype).nativeMethodFlags;
		if ((flags & PyCPeerType.SQ_ITEM) != 0 || (flags & PyCPeerType.SQ_LENGTH) != 0)
			return true;
		else return super.isSequenceType();
	}

/*
 * Writing the methods below makes me actually wish there was a (well established)
 * C-style preprocessor for Java. Yes, sometimes macros are evil, but sometimes they are
 * also really helpful, like in JyAbstract.c, where they prevented a lot of copy/paste-craft.
 */
// ------ implementation of PyNumber-methods ------
	@Override
	public PyObject __add__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_ADD) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Add(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_CONCAT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PySequence_Concat(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__add__(other);
	}

	@Override
	public PyObject __sub__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_SUBTRACT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Subtract(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__sub__(other);
	}

	@Override
	public PyObject __mul__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_MULTIPLY) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Multiply(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_REPEAT) != 0) {
			int l;
			try {
				l = other.asInt();
			} catch (Exception e) {
				return super.__mul__(other);
			}
			return JyNI.maybeExc(JyNI.JyNI_PySequence_Repeat(objectHandle, l,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		}
		return super.__mul__(other);
	}

	@Override
	public PyObject __div__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_DIVIDE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Divide(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__div__(other);
	}

	@Override
	public PyObject __mod__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_REMAINDER) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Remainder(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__mod__(other);
	}

	@Override
	public PyObject __divmod__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_DIVMOD) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Divmod(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__divmod__(other);
	}

	@Override
	public PyObject __pow__(PyObject o2, PyObject o3) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_POWER) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Power(objectHandle, o2, o3,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__pow__(o2, o3);
	}

	@Override
	public PyObject __neg__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_NEGATIVE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Negative(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__neg__();
	}

	@Override
	public PyObject __pos__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_POSITIVE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Positive(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__pos__();
	}

	@Override
	public PyObject __abs__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_ABSOLUTE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Absolute(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__abs__();
	}

	@Override
	public boolean __nonzero__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_NONZERO) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_NonZero(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		else return super.__nonzero__();
	}

	@Override
	public PyObject __invert__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INVERT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Invert(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		else return super.__invert__();
	}

	@Override
	public PyObject __lshift__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_LSHIFT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Lshift(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__lshift__(other);
	}

	@Override
	public PyObject __rshift__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_RSHIFT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Rshift(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__rshift__(other);
	}

	@Override
	public PyObject __and__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_AND) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_And(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__and__(other);
	}

	@Override
	public PyObject __xor__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_XOR) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Xor(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__xor__(other);
	}

	@Override
	public PyObject __or__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_OR) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Or(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__or__(other);
	}

	@Override
	public Object __coerce_ex__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_COERCE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Coerce(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__coerce_ex__(other);
	}

	@Override
	public PyObject __int__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Int(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__int__();
	}

	@Override
	public PyObject __long__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_LONG) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Long(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__long__();
	}

	@Override
	public PyFloat __float__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_FLOAT) != 0)
			return (PyFloat) JyNI.maybeExc(JyNI.JyNI_PyNumber_Float(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__float__();
	}

//	@Override
//	public PyString __oct__() {
//		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_OCT) != 0)
//			return (PyString) JyNI.maybeExc(JyNI.JyNI_PyNumber_Oct(objectHandle,
//					JyTState.prepareNativeThreadState(Py.getThreadState())));
//		return super.__oct__();
//	}
//
//	@Override
//	public PyString __hex__() {
//		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_HEX) != 0)
//			return (PyString) JyNI.maybeExc(JyNI.JyNI_PyNumber_Hex(objectHandle,
//					JyTState.prepareNativeThreadState(Py.getThreadState())));
//		return super.__hex__();
//	}

	@Override
	public PyObject __iadd__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_ADD) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceAdd(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_INPLACE_CONCAT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PySequence_InPlaceConcat(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__iadd__(other);
	}

	@Override
	public PyObject __isub__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_SUBTRACT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceSubtract(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__isub__(other);
	}

	@Override
	public PyObject __imul__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_MULTIPLY) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceMultiply(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_INPLACE_REPEAT) != 0) {
			int l;
			try {
				l = other.asInt();
			} catch (Exception e) {
				return super.__imul__(other);
			}
			return JyNI.maybeExc(JyNI.JyNI_PySequence_InPlaceRepeat(objectHandle, l,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		}
		return super.__imul__(other);
	}

	@Override
	public PyObject __idiv__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_DIVIDE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceDivide(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__idiv__(other);
	}

	@Override
	public PyObject __imod__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_REMAINDER) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceRemainder(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__imod__(other);
	}

	@Override
	public PyObject __ipow__(PyObject o2) { //this should have another arg o3, shouldn't it?
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_POWER) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlacePower(objectHandle, o2, null,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__ipow__(o2);
	}

	@Override
	public PyObject __ilshift__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_LSHIFT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceLshift(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__ilshift__(other);
	}

	@Override
	public PyObject __irshift__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_RSHIFT) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceRshift(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__irshift__(other);
	}

	@Override
	public PyObject __iand__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_AND) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceAnd(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__iand__(other);
	}

	@Override
	public PyObject __ixor__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_XOR) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceXor(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__ixor__(other);
	}

	@Override
	public PyObject __ior__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_OR) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceOr(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__ior__(other);
	}

	@Override
	public PyObject __floordiv__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_FLOOR_DIVIDE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_FloorDivide(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__floordiv__(other);
	}

	@Override
	public PyObject __truediv__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_TRUE_DIVIDE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_TrueDivide(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__truediv__(other);
	}

	@Override
	public PyObject __ifloordiv__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_FLOOR_DIVIDE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceFloorDivide(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__ifloordiv__(other);
	}

	@Override
	public PyObject __itruediv__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INPLACE_TRUE_DIVIDE) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_InPlaceTrueDivide(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__itruediv__(other);
	}

	@Override
	public PyObject __index__() {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.NB_INDEX) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyNumber_Index(objectHandle,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__index__();
	}


// ------ implementation of PySequence-methods ------
	@Override
	public PyObject __getitem__(int key) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_ITEM) != 0 &&
				(((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.MP_SUBSCRIPT) == 0)
			return JyNI.maybeExc(JyNI.JyNI_PySequence_GetItem(objectHandle, key,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__getitem__(key);
	}

	@Override
	public void __setitem__(int key, PyObject value) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_ASS_ITEM) != 0 &&
				(((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.MP_ASS_SUBSCRIPT) == 0)
			JyNI.maybeExc(JyNI.JyNI_PySequence_SetItem(objectHandle, key, value,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		else super.__setitem__(key, value);
	}

	@Override
	public PyObject __getslice__(PyObject start, PyObject end) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_SLICE) != 0) {
			int s, e;
			try {
				s = start.asInt();
				e = end.asInt();
			} catch (Exception ex) {
				return super.__getslice__(start, end);
			}
			return JyNI.maybeExc(JyNI.JyNI_PySequence_GetSlice(objectHandle, s, e,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		}
		return super.__getslice__(start, end);
	}

	@Override
	public void __setslice__(PyObject start, PyObject end, PyObject value) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_ASS_SLICE) != 0) {
			int s, e;
			try {
				s = start.asInt();
				e = end.asInt();
			} catch (Exception ex) {
				super.__setslice__(start, end, value);
				return;
			}
			JyNI.maybeExc(JyNI.JyNI_PySequence_SetSlice(objectHandle, s, e, value,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		}
		super.__setslice__(start, end, value);
	}

	@Override
	public boolean __contains__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.SQ_CONTAINS) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PySequence_Contains(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())), -1) != 0;
		return super.__contains__(other);
	}


// ------ implementation of PyMapping-methods ------
	@Override
	public PyObject __getitem__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.MP_SUBSCRIPT) != 0)
			return JyNI.maybeExc(JyNI.getItem(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__getitem__(other);
	}


// ------ implementation of iterator-related-methods ------
	@Override
	public PyObject __iter__() {
//		System.out.println("PyCPeer __iter__ "+getType().getName());
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_ITER) != 0)
			return JyNI.maybeExc(JyNI.JyNI_PyObject_GetIter(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		return super.__iter__();
	}

	@Override
	public PyObject __iternext__() {
		return JyNI.maybeExc(JyNI.JyNI_PyIter_Next(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
	}


// ------ implementation of comparison-methods ------
	@Override
	public int __cmp__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_COMPARE) != 0)
		{
			int res = JyNI.maybeExc(JyNI.JyNI_PyObject_Compare(objectHandle, other,
					JyTState.prepareNativeThreadState(Py.getThreadState())));
			return res;
		} else return super.__cmp__(other);
	}

	@Override
	public PyObject __eq__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_RICHCOMPARE) != 0)
		{
			return JyNI.maybeExc(JyNI.JyNI_PyObject_RichCompare(objectHandle, other,
					JyNI.Py_EQ, JyTState.prepareNativeThreadState(Py.getThreadState())));
		} else return super.__eq__(other);
    }

	@Override
	public PyObject __ne__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_RICHCOMPARE) != 0)
		{
			return JyNI.maybeExc(JyNI.JyNI_PyObject_RichCompare(objectHandle, other,
					JyNI.Py_NE, JyTState.prepareNativeThreadState(Py.getThreadState())));
		} else return super.__ne__(other);
    }

	@Override
	public PyObject __ge__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_RICHCOMPARE) != 0)
		{
			return JyNI.maybeExc(JyNI.JyNI_PyObject_RichCompare(objectHandle, other,
					JyNI.Py_GE, JyTState.prepareNativeThreadState(Py.getThreadState())));
		} else return super.__ge__(other);
    }

	@Override
    public PyObject __gt__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_RICHCOMPARE) != 0)
		{
			return JyNI.maybeExc(JyNI.JyNI_PyObject_RichCompare(objectHandle, other,
					JyNI.Py_GT, JyTState.prepareNativeThreadState(Py.getThreadState())));
		} else return super.__gt__(other);
    }

	@Override
	public PyObject __le__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_RICHCOMPARE) != 0)
		{
			return JyNI.maybeExc(JyNI.JyNI_PyObject_RichCompare(objectHandle, other,
					JyNI.Py_LE, JyTState.prepareNativeThreadState(Py.getThreadState())));
		} else return super.__le__(other);
    }

	@Override
    public PyObject __lt__(PyObject other) {
		if ((((PyCPeerType) objtype).nativeMethodFlags & PyCPeerType.OB_RICHCOMPARE) != 0)
		{
			return JyNI.maybeExc(JyNI.JyNI_PyObject_RichCompare(objectHandle, other,
					JyNI.Py_LT, JyTState.prepareNativeThreadState(Py.getThreadState())));
		} else return super.__lt__(other);
    }
}
