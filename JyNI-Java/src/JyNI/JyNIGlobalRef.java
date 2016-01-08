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

import org.python.core.Py;
import org.python.core.PyObject;
import org.python.core.PyList;
import org.python.modules._weakref.*;

public class JyNIGlobalRef implements ReferenceBackend {

	protected ReferenceBackend backend;
	protected long nativeHandle = 0;

//	public static JyNIGlobalRef makeJyNIGlobalRef(GlobalRef backend, long nativeHandle) {
//		if (backend.isCleared()) return new JyNIGlobalRef(nativeHandle);
//		if (backend instanceof JyNIGlobalRef) {
//			JyNIGlobalRef jb = (JyNIGlobalRef) backend;
//			if (nativeHandle == jb.nativeHandle)
//				return jb;
//			else return new JyNIGlobalRef(jb.backend, nativeHandle);
//		} else
//		return new JyNIGlobalRef(backend, nativeHandle);
//	}

//	protected void install() {
//		//Todo: Make this threadsafe.
//		if (backend == null) return;
//		PyList refs = backend.refs();
//		for (Object ref: refs) {
//			((AbstractReference) ref)._setBackend(this);
//		}
//		PyObject referent = backend.get();
//		if (referent != null)
//			JyAttribute.setAttr(referent, JyAttribute.WEAK_REF_ATTR, this);
//	}

//	protected JyNIGlobalRef(long nativeHandle) {
//		backend = null;
//		this.nativeHandle = nativeHandle;
//	}

	protected JyNIGlobalRef(GlobalRef backend) {//, long nativeHandle) {
		this.backend = backend;
		//this.nativeHandle = nativeHandle;
	}

	protected void initNativeHandle(long handle) {
		//System.out.println("initNativeHandle: "+handle+" (J"+System.identityHashCode(backend));
		//This should incref the native ref-count (if handle is non-null), shouldn't it?
		//For efficiency reasons, the native caller of JyNI.createWeakReferenceFromNative
		//should be responsible for the incref. Native calls to it should be strictly
		//controlled - currently only happening in JySync.c.
		nativeHandle = handle;
	}

	protected void notifyBackendClear(GlobalRef caller) {
		/* Decref native referent if present. */
//These four lines prevent JyNI from detecting a corrupted ref-graph properly:
//(todo: find out why!)
/* Notes: This should release the ref-count that keeps the native referent alive
          while this non-native ref exists. (So releasing the native side actually
          makes sense.) However somehow it can break/fix ref-graphs. Maybe the
          acquiring of the ref-count does not work reliably.
 */
		//Todo: Check/reason whether there might be issues if someone calls get while
		//      the weakref is in pending state regarding clear.
		//System.out.println("releaseWeakReferent? "+nativeHandle);
		long handle = nativeHandle;
		nativeHandle = 0;
		if (handle != 0) {
			//System.out.println(JyNI.currentNativeRefCount(handle));
			JyNI.releaseWeakReferent(handle,
					JyTState.prepareNativeThreadState(Py.getThreadState()));
		}
	}

	protected void notifyNativeClear() {
		nativeHandle = 0;
	}

	@Override
	public boolean isCleared() {
		return nativeHandle == 0 && (backend == null || backend.isCleared());
	}

//	protected PyObject restoreBackendFromNative() {
//		PyObject result = JyNI.JyNI.lookupFromHandle(nativeHandle);
//		if (result != null) {
//			backend.restore(result);
//			JyAttribute.setAttr(result, JyAttribute.WEAK_REF_ATTR, this);
//		}
//		return result;
//	}

	@Override
	public PyObject get() {
		//Todo: Make this thread-safe regarding GC.
		if (/*backend != null &&*/ !backend.isCleared())
			return backend.get(); //todo: make thread-safe with restore.
		if (nativeHandle == 0) return null;
		//return restoreBackendFromNative();
		PyObject result = JyNI.lookupFromHandle(nativeHandle);
		if (result != null && backend != null)
			backend.restore(result);
		//if (result == null) System.out.println("Obtained null-result from non-null native handle");
		return result;
//		return null;
	}

	public int pythonHashCode() {
//		if (backend != null) {
		try {
			return backend.pythonHashCode();
		} catch (org.python.core.PyException typeError) {}
//		}
		get();
//		if (backend != null) {
		return backend.pythonHashCode();
//		} else
//			throw Py.TypeError("weak object has gone away");
	}

	public void add(AbstractReference ref) {
		backend.add(ref);
//		if (backend != null) {
//			backend.add(ref);
//		} else {
//			restoreBackendFromNative();
//			if (backend != null) {
//				backend.add(ref);
//			}
//		}
	}

	public AbstractReference find(Class<?> cls) {
		return backend.find(cls);
//		if (backend != null) {
//			return backend.find(cls);
//		} else {
//			restoreBackendFromNative();
//			if (backend != null) {
//				return backend.find(cls);
//			}
//			return null;
//		}
	}

	public PyList refs() {
		return backend.refs();
//		if (backend != null) {
//			return backend.refs();
//		} else {
//			restoreBackendFromNative();
//			if (backend != null) {
//				return backend.refs();
//			}
//			return new PyList();
//		}
	}

	public void restore(PyObject formerReferent) {
		//System.out.println("restore WeakReferent: "+nativeHandle);
		backend.restore(formerReferent);
//		if (backend != null) {
//			backend.restore(formerReferent);
////			backend = (ReferenceBackend)
////					JyAttribute.getAttr(formerReferent, JyAttribute.WEAK_REF_ATTR);
////			JyAttribute.setAttr(formerReferent, JyAttribute.WEAK_REF_ATTR, this);
//		} else {
//			backend = GlobalRef.newInstance(formerReferent);
//			JyAttribute.setAttr(formerReferent, JyAttribute.WEAK_REF_ATTR, this);
//		}
	}

    public void updateBackend(ReferenceBackend backend) {
    	this.backend = backend;
    }

    public int count() {
    	return backend.count();
    }

//    public boolean isProxyFor(ReferenceBackend backend) {
//    	return this.backend == backend || this.backend.isProxyFor(backend);
//    }
}
