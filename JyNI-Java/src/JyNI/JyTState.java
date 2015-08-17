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

import java.util.WeakHashMap;
import java.lang.ref.WeakReference;

import org.python.core.Py;
//import org.python.core.PyException;
import org.python.core.ThreadState;

public class JyTState {
	protected static int nativeRecursionLimit;
	
	//No IdentityHashMap needed, since ThreadState does not overwrite hashCode().
	protected static WeakHashMap<ThreadState, JyTState> tStateLookup = new WeakHashMap<ThreadState, JyTState>();
	
	protected WeakReference<ThreadState> tState;
	protected int nativeCallDepth;
	protected long nativeHandle;

	public static void setRecursionLimit(int limit) {
		Py.getSystemState().setrecursionlimit(limit);
	}

	public static void setRecursionLimit(ThreadState ts, int limit) {
		ts.systemState.setrecursionlimit(limit);
	}

	public static JyTState fromThreadState(ThreadState ts) {
		JyTState res = tStateLookup.get(ts);
		if (res != null) return res;
		res = new JyTState(ts);
		tStateLookup.put(ts, res);
		return res;
	}

	public static long prepareNativeThreadState(ThreadState ts) {
		JyTState jts = fromThreadState(ts);
		syncToNative(jts);
		return jts.nativeHandle;
	}

	protected static void syncToNative(JyTState ts) {
		ThreadState tState = ts.tState.get();
		if (tState == null) return;
		if (nativeRecursionLimit != tState.systemState.getrecursionlimit())
		{
			nativeRecursionLimit = tState.systemState.getrecursionlimit();
			JyNI.setNativeRecursionLimit(nativeRecursionLimit);
		}
		
		if (ts.nativeCallDepth != tState.call_depth) {
			ts.nativeCallDepth = tState.call_depth;
			JyNI.setNativeCallDepth(ts.nativeHandle, ts.nativeCallDepth);
		}
	}

	protected JyTState(ThreadState tState)
	{
		this.tState = new WeakReference<ThreadState>(tState);
		nativeCallDepth = tState.call_depth;
		nativeHandle = JyNI.initNativeThreadState(this, tState);
//		System.out.println("JyTState created: "+nativeHandle);
	}
	
	protected void finalize() throws Throwable {
		JyNI.clearNativeThreadState(nativeHandle);
	}
}
