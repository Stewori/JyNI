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

import java.lang.ref.*;
import org.python.core.*;

public class JyReferenceMonitor {
//	0000 00## - 01=increase, 10=decrease, 11=realloc, 00=other
//	0000 0#00 - memory action vs reference action/other
//	0000 #000 - native action
//	000# 0000 - GC action
//	00#0 0000 - pre/start
//	0#00 0000 - post/finnish
//	#000 0000 - finalize action
	public static final short INC_MASK      =   1;
	public static final short DEC_MASK      =   2;
	public static final short MEMORY_MASK   =   4;
	public static final short NATIVE_MASK   =   8;
	public static final short GC_MASK       =  16;
	public static final short PRE_MASK      =  32;
	public static final short POST_MASK     =  64;
	public static final short FINALIZE_MASK = 128;
	public static final short INLINE_MASK   = 256;
	public static final short IMMORTAL_MASK = 512;

	public static final short NAT_ALLOC = INC_MASK | MEMORY_MASK | NATIVE_MASK;
	public static final short NAT_FREE = DEC_MASK | MEMORY_MASK | NATIVE_MASK;
	public static final short NAT_REALLOC = NAT_ALLOC | NAT_FREE;
	public static final short NAT_ALLOC_GC = NAT_ALLOC | GC_MASK;
	public static final short NAT_FREE_GC = NAT_FREE | GC_MASK;
	public static final short NAT_REALLOC_GC = NAT_REALLOC | GC_MASK;
	public static final short NAT_FINALIZE = NATIVE_MASK | FINALIZE_MASK | PRE_MASK;
	//i.e. expected to be called at the beginning of finalizer-code
	public static final short OBJECT_FINALIZE = FINALIZE_MASK | PRE_MASK;
	public static final short OBJECT_RESURRECT = FINALIZE_MASK | INC_MASK;
	public static final short OBJECT_DELETE = DEC_MASK | MEMORY_MASK;
	public static final short GC_RUN = GC_MASK | PRE_MASK;
	public static final short GC_PRE_FINALIZE = GC_MASK | FINALIZE_MASK | PRE_MASK;
	public static final short GC_POST_FINALIZE = GC_MASK | FINALIZE_MASK | POST_MASK;
	public static final short GC_DONE = GC_MASK | POST_MASK;

	public static String actionToString(short action) {
		StringBuilder result = new StringBuilder();
		if ((action & NATIVE_MASK) != 0) result.append("_NATIVE");
		if ((action & PRE_MASK) != 0) result.append("_PRE");
		if ((action & POST_MASK) != 0) result.append("_POST");
		if ((action & MEMORY_MASK) != 0) {
			if ((action & INC_MASK) != 0 && (action &  DEC_MASK) != 0) result.append("_REALLOC");
			else if ((action & (INC_MASK)) != 0) result.append("_ALLOC");
			else if ((action & (DEC_MASK)) != 0) result.append("_FREE");
		} else if ((action & GC_MASK) == 0) {
			if ((action & (INC_MASK)) != 0 && (action & (DEC_MASK)) != 0) result.append("_REF-RESTORE");
			else if ((action & (INC_MASK)) != 0) result.append("_INCREF");
			else if ((action & (DEC_MASK)) != 0) result.append("_DECREF");
		} else {
			if ((action & (INC_MASK)) != 0 && (action & (DEC_MASK)) != 0) result.append("_GC-RETRACK(??)");
			else if ((action & (INC_MASK)) != 0) result.append("_GC-TRACK");
			else if ((action & (DEC_MASK)) != 0) result.append("_GC-UNTRACK");
			else result.append("_GC");
		}
		//if ((action & GC_MASK) != 0) result.append("_GC");
		if ((action & INLINE_MASK) != 0) result.append("_INLINE");
		if ((action & FINALIZE_MASK) != 0 && (action & INC_MASK) != 0)  result.append("_RESURRECT");
		else if ((action & FINALIZE_MASK) != 0) result.append("_FINALIZE");
		if ((action & IMMORTAL_MASK) != 0) result.append("_IMMORTAL");
		return result.substring(1);
	}

	public static class MemAction {
		short action;
		long timestamp;

		public MemAction(short action) {
			this.action = action;
			timestamp = System.currentTimeMillis();
		}

		public MemAction(short action, long timestamp) {
			this.action = action;
			this.timestamp = timestamp;
		}
	}

	public static class PyObjectInfo {
		WeakReference<PyObject> ref;
		PyType type;
		String repr;
		int id;
		long nativeRef = 0;
		boolean nativeFreed;
		String nativeType;
		String comment;
	}

	public static class ObjectAction extends MemAction {
		PyObjectInfo info;

		public ObjectAction(short action, PyObjectInfo info) {
			super(action);
			this.info = info;
		}

		public ObjectAction(short action, long timestamp, PyObjectInfo info) {
			super(action, timestamp);
			this.info = info;
		}
	}

	public static void addAction(short action, PyObject obj) {
		
	}

	public static void addNativeAction(short action, PyObject obj, long nativeRef1, long nativeRef2,
			String nativeType, String cMethod, String file, int line) {
		System.out.println(actionToString(action)+" - "+action+" ("+cMethod+", "+nativeRef1+")");
		//System.out.println("addAction: "+action+" ("+cMethod+")");
	}
}
