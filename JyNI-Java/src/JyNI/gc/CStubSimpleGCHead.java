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


package JyNI.gc;

import JyNI.JyNI;
import JyNI.JyReferenceMonitor;
//import java.lang.ref.WeakReference;
import org.python.core.PyObject;
import org.python.modules.gc;

public class CStubSimpleGCHead extends SimpleGCHead implements PyObjectGCHead {
	private PyObject object;

	public CStubSimpleGCHead(long handle) {
		super(handle);
	}

	@Override
	public void setPyObject(PyObject object) {
//		System.out.println("CStubSimpleGCHead.setPyObject: "+System.identityHashCode(this)+" / "+handle+" "+System.identityHashCode(object));//object.getType().getName());
		this.object = object;
	}

	@Override
	public PyObject getPyObject() {
		//System.out.println("CStubSimpleGCHead.getPyObject: "+handle+nt);
		//if (obj != null) return (PyObject) ((WeakReference) obj).get();
		return object;
	}

//	String nt = "";
//	Object obj = null;
	@Override
	protected void finalize() throws Throwable {
//		System.out.println("CStubSimple finalize "+System.identityHashCode(this)+" "+object+" "+handle+" ("+JyNI.getNativeRefCount(object)+")");
//		System.out.println(Thread.currentThread().getName());
		gc.notifyPreFinalization();
		//System.out.println("CStubSimpleGCHead.finalize "+handle+nt);
		int result = JyNI.consumeConfirmation(handle);
		//System.out.println("consumeConfirmation done "+handle+nt);
		if ((result & JyNI.JYNI_GC_RESURRECTION_FLAG) != 0) {
			//System.out.println("Resurrect: "+handle+nt);
			CStubSimpleGCHead newHead = new CStubSimpleGCHead(handle);
			//newHead.nt = " (resurrected)";

			/*
			 * This line (the actual resurrection) for some strange reason
			 * causes the object to persist the next gc-cycle, not only
			 * the current one. And that without any notable further reference
			 * to it. We might have overlooked something, but everything we
			 * encountered hints to gc-responsibility.
			 *  - without changing anything the object is collected by one
			 *    gc-cycle later.
			 *  - Its gc-head is directly collected in the next cycle
			 *  - everything works as expected if we store a weak reference
			 *    instead (but this would not allow to handle repeated
			 *    occurrence of native silent modification and triggered
			 *    resurrection)
			 *  - getPyObject and setPyObject report no access between the
			 *    gc cycles (and actually not at all)
			 * So for now we assume that it is just natural gc-behavior that
			 * a resurrected object with a direct strong reference (overall
			 * just weakly reachable though) is kept alive for an additional
			 * gc-cycle (maybe it has to do with generation management etc).
			 * Since specification leaves these details undefined,
			 * this is still valid behavior and only delays collection by one
			 * cycle. Weak references hold somewhat longer, but apart from that
			 * it is no leak and causes no harm.
			 */
			newHead.object = object;

			//newHead.obj = new WeakReference(object1);
			JyNI.JyGC_restoreCStubBackend(handle, object, newHead);
			new JyWeakReferenceGC(newHead);
			JyNI.CStubRestoreAllReachables(object);
//			System.out.println("Resurrect CStubSimple "+object);
//			System.out.println(JyNI.lookupNativeHandle(object)+" ("+JyNI.getNativeRefCount(object)+")");
			JyReferenceMonitor.notifyResurrect(handle, object);
		} else if ((result & JyNI.JYNI_GC_CONFIRMED_FLAG) == 0) {
// We make this temporarily silent. Todo: Investigate warnings!
			//System.err.println("JyNI-Warning: Unconfirmed but finalized CStubSimpleGCHead: "+handle);
			//System.err.println("  "+object1);
		}
		//object = null;  //(also makes no difference)
		if ((result & JyNI.JYNI_GC_LAST_CONFIRMATION_FLAG) != 0)
			JyNI.postProcessCStubGCCycle();
		//System.out.println("CStubSimpleGCHead.finalize done "+handle+nt);
		gc.notifyPostFinalization();
	}
}
