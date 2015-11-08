package JyNI;

import java.util.HashMap;

import org.python.core.Py;
import org.python.core.PyDictionary;
import org.python.core.PyObject;
import org.python.core.PyString;
import org.python.core.PyTuple;
import org.python.core.finalization.FinalizableBuiltin;

/**
 * Specialized CPeer classes are used to wrap native custom types that extend
 * builtin types. For consistency the Java-counterpart must also be something
 * that extends the Java-counterpart of the builtin. For now we don't support
 * nesting of this principle.
 *
 * PyDictionary is a pilot project on this front. Once we have implemented some
 * sample builtin-extending CPeer classes we will work out a generator script
 * like the one that generates the PyFooDerived-classes in Jython.
 * Such a script will basically have two modi:
 * One that implements magic methods entirely on top of native methods (e.g.
 * like in PyCPeerType) and one that uses Jython-builtin super-methods as
 * fallbacks, if a native one fails (like in PyDictionaryCPeer).
 *
 * We support PyDictionary with priority, because it is needed for ctypes to
 * wrap stgdict properly.
 *
 * Whenever we call a native method we interpret a null-result not accompanied
 * by an exception as a not-implemented hint. In that case we try the super method,
 * which is responsible for throwing a not-implemented exception if needed. This
 * way we emulate that native methods would override thuse from super-class.
 * Todo: Clean up this concept for int-returning methods, etc.
 *
 * @author Stefan Richthofer
 */
public class PyDictionaryCPeer extends PyDictionary implements
		CPeerNativeDelegateSubtype, FinalizableBuiltin {

	public long objectHandle;

	/**
	 * This constructor signature is obligatory for every specialized
	 * CPeer-class.
	 */
	public PyDictionaryCPeer(long objectHandle, PyCPeerType subtype) {
		super(subtype);
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
}
