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
 *
 * We support PyDictionary with priority, because it is needed for ctypes to
 * wrap stgdict properly.
 *
 * Whenever we call a native method we interpret a null-result not accompanied
 * by an exception as a not-implemented hint. In that case we try the super method,
 * which is responsible for throwing a not-implemented exception if needed. This
 * way we emulate that native methods would override thuse from super-class.
 *
 * @author Stefan Richthofer
 */
public class PyDictionaryCPeer extends PyDictionary implements
		CPeerInterface, FinalizableBuiltin {
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

	public PyObject __findattr_ex__(String name) {
		//System.out.println("Look for attribute "+name+" in PyCPeerType "+this.name+" "+(findAttrCount++));
		long ts = JyTState.prepareNativeThreadState(Py.getThreadState());
		PyObject result = JyNI.maybeExc(JyNI.getAttrString(objectHandle, name, ts));
		return result != null ? result : super.__findattr_ex__(name);
	}

	public PyString __str__() {
		PyString result = (PyString) JyNI.maybeExc(JyNI.PyObjectAsPyString(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		if (result == null) result = (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		return result == null ? super.__str__() : result;
	}

	public PyString __repr__() {
		PyString result = (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
			JyTState.prepareNativeThreadState(Py.getThreadState())));
		return result == null ? super.__repr__() : result;
	}

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
}
