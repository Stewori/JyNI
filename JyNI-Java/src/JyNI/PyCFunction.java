package JyNI;

import java.util.HashMap;

import org.python.core.*;
import org.python.core.finalization.FinalizableBuiltin;
import org.python.expose.ExposedType;

import JyNI.gc.DefaultTraversableGCHead;
import JyNI.gc.JyGCHead;
import JyNI.gc.JyVisitproc;
import JyNI.gc.TraversableGCHead;

@Untraversable
public class PyCFunction extends PyBuiltinCallable implements CPeerInterface,
		FinalizableBuiltin, TraversableGCHead {

	public long objectHandle;
	protected Object links;

	protected PyCFunction(long objectHandle, PyType subtype, String name, boolean noArgs, String doc) {
		super(subtype, new DefaultInfo(name, 0, noArgs ? 0 : -1));
		this.objectHandle = objectHandle;
		this.doc = doc;
//		System.out.println("New PyCFunction: "+name);
	}

	@Override
	public PyBuiltinCallable bind(PyObject self) {
		return this;
	}

	@Override
    public PyObject getModule() {
		PyObject result = JyNI.maybeExc(JyNI.PyCFunction_getModule(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		return result != null ? result : Py.None;
    }

	@Override
    public PyObject getSelf() {
//		System.out.println("getSelf...");
		return JyNI.maybeExc(JyNI.PyCFunction_getSelf(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
//		PyObject result = JyNI.PyCFunction_getSelf(objectHandle,
//				JyTState.prepareNativeThreadState(Py.getThreadState()));
//		System.out.println("got result:");
//		System.out.println(result);
//		return result;// != null ? result : Py.None;
    }

	/**
	 * Copied from PyCPeer.
	 */
	@Override
	public PyObject __call__(PyObject[] args, String[] keywords) {
//		System.out.println("PyCFunction.__call__ "+info.getName()+" "+System.identityHashCode(this));
//		System.out.println("args: "+args.length+" kws: "+keywords.length);
//    	System.out.println("kws:");
//    	for (String kw: keywords) System.out.println(kw);
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
//				System.out.println("call PyCPeer... kw "+back.size());
//				for (PyObject ky: back.keySet()) System.out.println(ky);
//				System.out.println("--------------");
				result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle, new PyTuple(args2, false),
					new PyDictionary(back), JyTState.prepareNativeThreadState(Py.getThreadState())));
			} else {
//				System.out.println("call PyCPeer2... kw "+back.size());
//				for (PyObject ky: back.keySet()) System.out.println(ky);
//				System.out.println("--------------");
				result = JyNI.maybeExc(JyNI.callPyCPeer(objectHandle, Py.EmptyTuple,
					new PyDictionary(back), JyTState.prepareNativeThreadState(Py.getThreadState())));
			}
		}
		if (result == null) {
			// This is equivalent to
			// throw Py.TypeError(String.format("'%s' object is not callable", getType().fastGetName()));
			// since parent is PyObject. Using super the error message automatically reflects updates
			// to Jython-code.
			return super.__call__(args, keywords);
		} else
			return result;
	}

	@Override
	public PyObject __findattr_ex__(String name) {
//		System.out.println("Look for attribute "+name+" in PyCFunction");
		PyObject er = JyNI.maybeExc(JyNI.getAttrString(objectHandle, name,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
		return er != null ? er : Py.None;
	}

	@Override
	public PyString __repr__() {
		return (PyString) JyNI.maybeExc(JyNI.repr(objectHandle,
				JyTState.prepareNativeThreadState(Py.getThreadState())));
//		System.out.println("J__repr__ "+res);
//		return res;
	}

	@Override
	public void __del_builtin__() {
		//System.out.println("PyCFunction finalize: "+objectHandle);
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, 0);
	}

	@Override
	public long getHandle() {
		return objectHandle;
	}


/* ---- implementation of GC-section, copied from PyCPeerGC ---- */

	public void setLinks(Object links) {
		this.links = links;
	}

	@Override
	public int setLink(int index, JyGCHead link) {
		int result = DefaultTraversableGCHead.setLink(links, index, link);
		if (result == 1) {
			links = link;
			return 0;
		}
		return result;
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

	@Override
	public void ensureSize(int size) {
		links = DefaultTraversableGCHead.ensureSize(links, size);
	}

	@Override
	public void printLinks(java.io.PrintStream out) {
		DefaultTraversableGCHead.printLinksAsHashes(links, out);
	}
}
