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


package JyNI.gc;

import JyNI.JyNI;
import JyNI.JyReferenceMonitor;
import org.python.core.PyObject;
import org.python.modules.gc;

public class CStubGCHead extends DefaultTraversableGCHead implements PyObjectGCHead {
	protected PyObject object;

	public CStubGCHead(long handle) {
		super(handle);
	}

	@Override
	public void setPyObject(PyObject object) {
//		System.out.println("CStubGCHead.setPyObject");
		this.object = object;
	}

	@Override
	public PyObject getPyObject() {
		return object;
	}

	@Override
	protected void finalize() throws Throwable {
		gc.notifyPreFinalization();
		//System.out.println("CStubGCHead.finalize "+handle);
		int result = JyNI.consumeConfirmation(handle);
		//System.out.println("consumeConfirmation done "+handle);
		if ((result & JyNI.JYNI_GC_RESURRECTION_FLAG) != 0) {
			CStubGCHead newHead = new CStubGCHead(handle);
			newHead.object = object;
			newHead.gclinks = gclinks;
			JyNI.JyGC_restoreCStubBackend(handle, object, newHead);
			new JyWeakReferenceGC(newHead);
//			System.out.println("Resurrect CStub "+object);
			JyNI.CStubRestoreAllReachables(object);
			JyReferenceMonitor.notifyResurrect(handle, object);
		} else if ((result & JyNI.JYNI_GC_CONFIRMED_FLAG) == 0) {
// We make this temporarily silent. Todo: Investigate warnings!
			//System.err.println("JyNI-Warning: Unconfirmed but finalized CStubGCHead: "+handle);
			//System.err.println("  "+object);
		}
		if ((result & JyNI.JYNI_GC_LAST_CONFIRMATION_FLAG) != 0)
			JyNI.postProcessCStubGCCycle();
		//System.out.println("CStubGCHead.finalize done "+handle);
		gc.notifyPostFinalization();
	}
}
