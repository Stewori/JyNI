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


package JyNI.gc;

import org.python.core.Py;
import JyNI.JyNI;
import JyNI.JyTState;
import JyNI.JyReferenceMonitor;
import java.lang.ref.WeakReference;
import java.lang.ref.ReferenceQueue;
import java.util.ArrayList;
import java.util.HashMap;

public class JyWeakReferenceGC extends WeakReference<JyGCHead> {
	public static boolean nativecollectionEnabled = true;
	public static boolean monitorNativeCollection = false;
	protected static ReferenceQueue<JyGCHead> refQueue = new ReferenceQueue<>();
	protected static HashMap<Long, JyWeakReferenceGC> refList = new HashMap<>();
	protected static GCReaperThread reaper = new GCReaperThread();

	protected long nativeRef;

	public JyWeakReferenceGC(JyGCHead head) {
		super(head, refQueue);
		nativeRef = head.getHandle();
		if (nativeRef != 0) {
			//System.out.println("Create JyWeakRef: "+nativeRef);
			//System.out.println(JyNI.lookupFromHandle(nativeRef));
			JyReferenceMonitor.addJyWeakRef(nativeRef);
		} //else System.out.println("Create JyWeakRef: sentinel");
		refList.put(nativeRef, this);
//		if (reaper == null) {
//		reaper = new GCReaperThread();
//		reaper.start();
	}

	public static JyWeakReferenceGC lookupJyGCHead(long handle) {
		return refList.get(handle);
		//return result != null ? result.get() : null;
	}

	static class GCReaperThread extends Thread {

		public GCReaperThread() {
			super("JyNI-GCRefReaper");
			setDaemon(true);
			start();
		}

		public void run() {
			ArrayList<JyWeakReferenceGC> refCache = new ArrayList<>(100);
			JyWeakReferenceGC ref;
			int pos;
			long[] clearRefs;
			while (true) {
				//System.out.println("GC-reaper cycle");
				try {
					ref = (JyWeakReferenceGC) refQueue.remove();
					if (ref.nativeRef != 0) refCache.add(ref);
					//else System.out.println("sentinel0");
					Thread.sleep(500); //Todo: Find a cleaner solution here.
				} catch(InterruptedException ie) {} //never happens
				//We try to chunk some refs here to reduce native calls.
				ref = (JyWeakReferenceGC) refQueue.poll();
				while (ref != null) {
					if (ref.nativeRef != 0) refCache.add(ref);
					//else System.out.println("sentinel1");
					ref = (JyWeakReferenceGC) refQueue.poll();
				}
				JyReferenceMonitor.notifyGCRun();
				if (!refCache.isEmpty()) {
					clearRefs = new long[refCache.size()];
					pos = 0;
					//System.out.println("Attempt native clear:");
					for (JyWeakReferenceGC ref0: refCache) {
						refList.remove(ref0.nativeRef);
						JyReferenceMonitor.clearJyWeakRef(ref0.nativeRef);
						clearRefs[pos++] = ref0.nativeRef;
						//System.out.println("  "+JyReferenceMonitor.getLog(ref0.nativeRef));
						//System.out.println("  "+ref0.nativeRef+" "+JyNI.lookupFromHandle(ref0.nativeRef));
					}
					refCache.clear();
					if (monitorNativeCollection) {
						for (long l: clearRefs)
							JyReferenceMonitor.notifyJyNIFree(l);
					}
					if (nativecollectionEnabled) {
						//System.out.println("Attempt native clear...");
						boolean validGraph = JyNI.JyGC_clearNativeReferences(clearRefs,
								JyTState.prepareNativeThreadState(Py.getThreadState()));
						//System.out.println("native clear done "+validGraph);
						JyReferenceMonitor.notifyClearReferences(clearRefs, validGraph);
					}
				}
			}
		}
	}
}
