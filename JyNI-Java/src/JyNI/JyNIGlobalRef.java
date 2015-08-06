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

//import org.python.core.JyAttribute;
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
		nativeHandle = handle;
	}

	protected void notifyBackendClear(GlobalRef caller) {
		/* Decref native referent if present. */
		if (nativeHandle != 0) {
			JyNI.releaseWeakReferent(nativeHandle,
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
		return result;
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
