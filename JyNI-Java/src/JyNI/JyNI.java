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

import JyNI.gc.*;
import org.python.core.*;
import org.python.core.finalization.FinalizeTrigger;
import org.python.modules.gc;
import org.python.modules._weakref.GlobalRef;
import org.python.modules.gc.CycleMarkAttr;

import java.lang.reflect.Field;
import java.util.*;
import java.io.File;

public class JyNI {
	static {
		try {
			/* To make configuration easier, we not only search on the library-path for the libs,
			 * but also on the classpath and in some additional typical places.
			 * We currently don't look inside jar files.
			 */
			String classpath = System.getProperty("java.class.path");
			String libpath = System.getProperty("java.library.path");
//			System.out.println("lp: "+libpath);
			String[] commonPositions = (libpath+File.pathSeparator+classpath).split(File.pathSeparator);
			int idx;
			for (int i = 0; i < commonPositions.length; ++i)
			{
				if (commonPositions[i].endsWith(".jar"))
				{
					idx = commonPositions[i].lastIndexOf('/');
					//System.out.println("com: "+commonPositions[i]);
					if (idx > -1)
						commonPositions[i] = commonPositions[i].substring(0, idx);
					else
						commonPositions[i] = System.getProperty("user.dir");
				}
				//System.out.println(commonPositions[i]);
			}
			//these are relative paths to typical positions where IDEs place their build-results:
			String[] loaderPositions = {".", "bin", "../JyNI-Loader/Release", "../JyNI-Loader/Debug"};
			String[] libPositions = {".", "bin", "../JyNI-C/Release", "../JyNI-C/Debug"};
			String loader = "libJyNI-Loader.so";
			String lib = "libJyNI.so";
			
			boolean loaded = false;
			String dir = System.getProperty("user.dir");
			//System.out.println("user.dir: "+dir);
			idx = dir.lastIndexOf('/');
			if (idx >= 0) dir = dir.substring(0, idx);
			else dir = System.getProperty("user.dir");
			String[] fileNames = new String[commonPositions.length+loaderPositions.length];
			int pos = 0;
			for (int i = 0; i < commonPositions.length; ++i)
			{
				fileNames[pos++] = commonPositions[i]+"/"+loader;
			}
			for (int i = 0; i < loaderPositions.length; ++i)
			{
				fileNames[pos++] = dir+"/"+loaderPositions[i]+"/"+loader;
			}
			
			
			for (int i = 0; !loaded && i < fileNames.length; ++i)
			{
				File loaderFile = new File(fileNames[i]);
				if (loaderFile.exists())
				{
					System.load(loaderFile.getAbsolutePath());
					loaded = true;
				} //else
					//System.out.println("not found: "+loaderFile.getPath());
			}
			if (!loaded)
			{
				System.err.print("Can't find library file: "+loader);
				System.exit(1);
			}
			
			
			fileNames = new String[commonPositions.length+libPositions.length];
			pos = 0;
			for (int i = 0; i < commonPositions.length; ++i)
			{
				fileNames[pos++] = commonPositions[i]+"/"+lib;
			}
			for (int i = 0; i < libPositions.length; ++i)
			{
				fileNames[pos++] = dir+"/"+libPositions[i]+"/"+lib;
			}
			
			loaded = false;
			for (int i = 0; !loaded && i < fileNames.length; ++i)
			{
				File libFile = new File(fileNames[i]);
				if (libFile.exists())
				{
					//System.out.println("initJyNI: "+fileNames[i]);
					//nativeHandles = new IdentityHashMap<PyObject, Long>();
					//cur_excLookup = new IdentityHashMap<ThreadState, PyException>(5);
					initJyNI(libFile.getAbsolutePath());
					//System.out.println("initJyNI done");
					loaded = true;
				}
			}
			if (!loaded)
			{
				System.err.print("Can't find library file: "+lib);
				System.exit(1);
			}
		} catch (Exception ex)
		{
			System.err.println("JyNI: Exception in initializer: "+ex);
		}
	}
	
	public static final String JyNIHandleAttr = "_JyNIHandleAttr".intern();
	
	/**Lazy function call binding.*/
	public static final int RTLD_LAZY = 0x00001;
	/**Immediate function call binding.*/
	public static final int RTLD_NOW = 0x00002;
	/**Mask of binding time value.*/
	public static final int RTLD_BINDING_MASK = 0x3;
	/**Do not load the object.*/
	public static final int RTLD_NOLOAD = 0x00004;
	/**Use deep binding.*/
	public static final int RTLD_DEEPBIND = 0x00008;

	/**If the following bit is set in the MODE argument to `dlopen',
	   the symbols of the loaded object and its dependencies are made
	   visible as if the object were linked directly into the program.*/
	public static final int RTLD_GLOBAL = 0x00100;

	/**Unix98 demands the following flag which is the inverse to RTLD_GLOBAL.
	   The implementation does this by default and so we can define the
	   value to zero.*/
	public static final int RTLD_LOCAL = 0;

	/** Do not delete object when closed.*/
	public static final int RTLD_NODELETE = 0x01000;

	public static final int RTLD_JyNI_DEFAULT = RTLD_LAZY | RTLD_GLOBAL;//RTLD_NOW;

	//Note: nativeHandles keeps (exclusively) natively needed objects save from beeing gc'ed.
	//protected static IdentityHashMap<PyObject, PyObject> nativeHandlesKeepAlive = new IdentityHashMap<>();

	/*
	 * This keeps alive objects (especially CStub-backends) reachable from PyObjects
	 * allocated on the C-stack rather than on the heap.
	 */
	public static HashMap<Long, JyGCHead> nativeStaticPyObjectHeads = new HashMap<>();
	public static Set<Long> JyNICriticalObjectSet = new HashSet<>();

	/*
	 * This is to keep the backend of the native interned string-dict alive.
	 * Todo: Arrange that this is a PyStringMap rather than PyDictionary.
	 * Todo: Join this with nativeStaticPyObjectHeads using a CStub(Simple?)GCHead.
	 */
	public static PyObject nativeInternedStrings = null;
	
	//protected static IdentityHashMap<PyObject, Long> nativeHandles;// = new HashMap<PyObject, Long>();
	//protected static IdentityHashMap<ThreadState, PyException> cur_excLookup;
	/*
	 * Todo: Make this a weak HashMap to allow PyCPeers to be mortal.
	 */
	protected static HashMap<Long, PyObject> CPeerHandles = new HashMap<Long, PyObject>();

	public static native void initJyNI(String JyNILibPath);
	public static native void clearPyCPeer(long objectHandle, long refHandle);
	public static native PyModule loadModule(String moduleName, String modulePath, long tstate);
	//public static native JyObject callModuleFunctionGlobalReferenceMode(CPythonModule module, String name,
			//JyObject self, JyObject... args);
	//public static native PyObject callModuleFunctionGlobalReferenceMode(JyNIModule module, String name,
			//PyObject self, long selfNativeHandle, PyObject[] args, long[] handles);
	//public static native PyObject callModuleFunctionLocalReferenceMode(JyNIModule module, String name,
			//PyObject self, long selfNativeHandle, PyObject... args);
	public static native PyObject callPyCPeer(long peerHandle, PyObject args, PyObject kw, long tstate);
	public static native PyObject getAttrString(long peerHandle, String name, long tstate);
	public static native int setAttrString(long peerHandle, String name, PyObject value, long tstate);
	public static native PyObject repr(long peerHandle, long tstate);
	public static native String PyObjectAsString(long peerHandle, long tstate);
	public static native PyString PyObjectAsPyString(long peerHandle, long tstate);
	public static native PyObject lookupFromHandle(long handle);
	public static native int currentNativeRefCount(long handle);
	public static native String getNativeTypeName(long handle);

	//ThreadState-stuff:
	public static native void setNativeRecursionLimit(int nativeRecursionLimit);
	public static native void setNativeCallDepth(long nativeHandle, int callDepth);
	public static native long initNativeThreadState(JyTState jts, ThreadState ts);
	public static native void clearNativeThreadState(long nativeHandle);

	public static native void JyNIDebugMessage(long mode, long value, String message);

	//List-Stuff:
	public static native PyObject JyList_get(long handle, int index);
	public static native int JyList_size(long handle);
	public static native PyObject JyList_set(long handle, int index, PyObject o, long pyObject);
	public static native void JyList_add(long handle, int index, PyObject o, long pyObject);
	public static native PyObject JyList_remove(long handle, int index);

	//Set-Stuff:
	public static native void JySet_putSize(long handle, int size);

	//ReferenceMonitor- and GC-Stuff:
	public static native void JyRefMonitor_setMemDebugFlags(int flags);

	/**
	 * Returns true, if the whole graph could be deleted (valid graph).
	 * Returns false, if at least one object had to be resurrected (invalid graph).
	 */
	public static native boolean JyGC_clearNativeReferences(long[] nativeRefs, long tstate);

	/**
	 * Note that this method won't acquire the GIL, because it is only called while
	 * JyGC_clearNativeReferences is holding the GIL for it anyway.
	 * JyGC_clearNativeReferences waits (via waitForCStubs) until all CStub-finalizers
	 * are done.
	 */
	public static native void JyGC_restoreCStubBackend(long handle, PyObject backend, JyGCHead newHead);
	//public static native long[] JyGC_validateGCHead(long handle, long[] oldLinks);
	public static native boolean JyGC_validateGCHead(long handle, long[] oldLinks);
	public static native long[] JyGC_nativeTraverse(long handle);
	//public static native JyGCHead JyGC_lookupGCHead(long handle);

	//use PySet.set_pop() instead. There are also hidden direct correspondents to other set methods.
	/*public static PyObject PySet_pop(BaseSet set)
	{
		if (set.isEmpty()) return null;
		Iterator it = set.iterator();
		Object obj = it.next();
		
		PyObject er = (PyObject) obj;//it.next();
		System.out.println("PySet_pop retrieves "+er.__repr__());
//		while (!set.remove(er))
//		{
//			try {er = (PyObject) it.next();}
//			catch (Exception e) {it = set.iterator();}
//		}
//		System.out.println("PySet_pop returns "+er.__repr__());
		return er;
	}*/

//	public static PyObject callModuleFunction(JyNIModule module, String name, PyObject self, long selfNativeHandle, PyObject... args)
//	{
//		return callModuleFunctionLocalReferenceMode(module, name, self, selfNativeHandle, args);
//	}

	public static PyObject getPyObjectByName(String name)
	{
		//todo: Check, whether this does what it is supposed to
		//System.out.println("JyNI: Getting Object by name: "+name);
		PySystemState sysState = Py.getSystemState();
		return Py.getThreadState().systemState.__dict__.__getitem__(new PyString(name))._doget(sysState);
		//PyObject er = Py.getThreadState().systemState.__dict__.__getitem__(new PyString(name));
		/*PyObject er = sysState.__dict__.__finditem__(name);
		System.out.println("found: "+er.getClass().getName());
		System.out.println(er.getType());
		System.out.println(er.getType().getClass().getName());
		System.out.println(er.getType().getName());
		PyObject er2 = er._doget(sysState);
		System.out.println(er2);
		System.out.println(er2.getClass());
		System.out.println(er2.getType());
		return er;*/
	}

	public static long getJyObjectByName(String name)
	{
		//System.out.println("getJyObjectByName "+name);
		PyObject obj = getPyObjectByName(name);
		//System.out.println("obj: "+obj);
		//Long er = nativeHandles.get(obj);
		//System.out.println("nativeHandles: "+nativeHandles);
		//System.out.println("found: "+er);
		//return er != null ? er : 0;
		return lookupNativeHandle(obj);
	}

	public static int getDLVerbose()
	{
		//current hard-coded Debug:
		return 0;
		
		//real implementation:
		/*switch (Options.verbose)
		case Py.DEBUG: return true;
		case Py.COMMENT: return true;
		case Py.MESSAGE: return true;
		default: return false;*/
	}

	public static int getDLOpenFlags()
	{
		try {
			return ((PyInteger) Py.getThreadState().systemState.__getattr__("dlopenflags")).asInt();
		} catch (Exception e1)
		{
			try {
				return ((PyInteger) Py.getSystemState().__getattr__("dlopenflags")).asInt();
			} catch (Exception e2)
			{
				return RTLD_JyNI_DEFAULT;
			}
		}
	}
		
	public static void setDLOpenFlags(int n)
	{
		try {
			Py.getThreadState().systemState.__setattr__("dlopenflags", new PyInteger(n));
		} catch (Exception e1)
		{
			try {
				Py.getSystemState().__setattr__("dlopenflags", new PyInteger(n));
			} catch (Exception e2)
			{
				return;
			}
		}
	}

//	public static void registerNativeStaticTypeDict(String type, PyDictionary dict) {
//		//System.out.println("JyNI Registered type-dict: "+type+" - "+lookupNativeHandle(dict));
//		if (nativeStaticTypeDicts.get(type) != null)
//			System.out.println("Warning: Registered type-dict twice: "+type);
//		nativeStaticTypeDicts.put(type,  dict);
//	}

	public static void registerNativeStaticJyGCHead(long handle, JyGCHead head) {
		nativeStaticPyObjectHeads.put(handle, head);
	}

	public static JyGCHead getNativeStaticJyGCHead(long handle) {
		return nativeStaticPyObjectHeads.get(handle);
	}

	public static void addJyNICriticalObject(long handle) {
		JyNICriticalObjectSet.add(handle);
	}

	public static void removeJyNICriticalObject(long handle) {
		JyNICriticalObjectSet.remove(handle);
	}

	public static void setNativeHandle(PyObject object, long handle) {//, boolean keepAlive) {
		//no WeakReferences needed here, because clearNativeHandle is always called
		//when a corresponding PyObject on C-Side is deallocated

		//todo: When JyNI-gc is stable remove keepAlive mechanism and option here.
		//Was only needed for type-dicts of static types, which don't get JyGCHeads.
		//Now nativeStaticTypeDicts serves this purpose more explicitly.
//		if (keepAlive) {
//			nativeHandlesKeepAlive.put(object, object);
//			//System.out.println("Would keep alive: "+handle+" - "+object);
//		}
		if (object instanceof PyCPeer) {
			((PyCPeer) object).objectHandle = handle;
		} else {
			JyAttribute.setAttr(object, JyAttribute.JYNI_HANDLE_ATTR, handle);
		}
	}

	//public static long lookupNativeHandle(PyObject object)
	public static long lookupNativeHandle(PyObject object) {
		//System.out.println("lookup native handle: "+object);
		if (object == null) return 0;
		if (object instanceof PyCPeer) return ((PyCPeer) object).objectHandle;
		else {
			//Long er = nativeHandles.get(object);
			Long er = (Long) JyAttribute.getAttr(object, JyAttribute.JYNI_HANDLE_ATTR);
			return er == null ? 0 : er.longValue();

//			System.out.println("Exception before:");
//			System.out.println(Py.getThreadState().exception);
			//We abuse PyCPeer as a dumb container for the native handle here.
			//System.out.println("find handle...");
			//PyCPeer peer = (PyCPeer) object.__findattr__(JyNIHandleAttr);
//			System.out.println("Exception afterwards:");
//			System.out.println(Py.getThreadState().exception);
//			System.out.println("retrieved: "+peer);
			//return peer == null ? 0 : ((PyCPeer) peer).objectHandle;
		}
		//Problem: When JyNI is initialized, nativeHandles have not been initialized...
		/*try
		{
			Long er = nativeHandles.get(object);
			return er == null ? 0 : er;
		} catch (Exception e)
		{
			System.out.println("ERRR: "+e);
			//System.out.println(nativeHandles);
			System.out.println(object);
		}
		return 0;*/
	}

	public static PyObject lookupCPeerFromHandle(long handle) {
		//Problem: When JyNI is initialized, nativeHandles have not been initialized...
		if (handle == 0) return null;
		else return CPeerHandles.get(handle);
	}

	public static void clearNativeHandle(PyObject object) {
		if (object == null) {
			System.err.println("JyNI-Warning: clearNativeHandle called with null!");
			return;
		}
		//System.out.println("java clearNativeHandle:");
		//System.out.println(object);
		if (object instanceof PyCPeer)
			((PyCPeer) object).objectHandle = 0;
		else
			JyAttribute.delAttr(object, JyAttribute.JYNI_HANDLE_ATTR);
			//nativeHandles.remove(object);
		//nativeHandlesKeepAlive.remove(object);
			
//		PyCPeer peer = (PyCPeer) object.__findattr__(JyNIHandleAttr);
//		if (peer != null)
//		{
//			peer.objectHandle = 0;
//			object.__delattr__(JyNIHandleAttr);
//		}
	}

//	public static PyObject callModuleFunctionGlobalReferenceMode(JyNIModule module, String name, PyObject self, PyObject... args)
//	{
//		Long selfHandle = nativeHandles.get(self);
//		
//		long[] handles = new long[args.length];
//		for (int i = 0; i < args.length; ++i)
//		{
//			Long handle = nativeHandles.get(args[i]);
//			handles[i] = handle == null ? 0 : handle;
//		}
//		return callModuleFunctionGlobalReferenceMode(module, name, self, selfHandle == null ? 0 : selfHandle, args, handles);
//	}
	
	public static PyObject _PyImport_FindExtension(String name, String filename) {
		return null;
	}
	
	public static PyObject PyImport_GetModuleDict() {
		return Py.getSystemState().modules;
	}
	
	public static PyObject PyImport_AddModule(String name) {
		String nm = name.intern();
		PySystemState pss = Py.getSystemState();
		PyObject er = pss.modules.__finditem__(name);
		if (er != null && er.getType().isSubType(PyModule.TYPE)) return er;
		else
		{
			er = new PyModule(nm);
			//pss.modules.__setattr__(nm, er);
			pss.modules.__setitem__(name, er);
			//System.out.println("JYNY rr: "+er);
			//System.out.println(er.getType().getName());
			//ERRR
			return er;
		}
	}
	
	public static PyObject JyNI_GetModule(String name) {
		String nm = name.intern();
		PySystemState pss = Py.getSystemState();
		PyObject er = pss.modules.__finditem__(name);
		if (er != null && er.getType().isSubType(PyModule.TYPE)) return er;
		else {
			System.out.println("JyNI: No module found: "+name);
			return null;
		}
	}
	
	public static PyType getPyType(Class pyClass) {
		try {
			Field tp = pyClass.getField("TYPE");
			PyType t = (PyType) tp.get(null);
			//System.out.println(t.getName());
			return t;
		} catch (Exception e) {return null;}
	}
	
	public static long[] getNativeAvailableKeysAndValues(PyDictionary dict) {
		Map<PyObject, PyObject> map = dict.getMap();
		Iterator<PyObject> it = map.keySet().iterator();
		long l;
		Vector<Long> er = new Vector<Long>();
		while (it.hasNext()) {
			//l = nativeHandles.get(it.next());
			//if (l != null) er.add(l);
			l = lookupNativeHandle(it.next());
			if (l != 0 ) er.add(l);
		}
		it = map.values().iterator();
		while (it.hasNext()) {
			//l = nativeHandles.get(it.next());
			//if (l != null) er.add(l);
			l = lookupNativeHandle(it.next());
			if (l != 0 ) er.add(l);
		}
		long[] er2 = new long[er.size()];
		for (int i = 0; i < er2.length; ++i)
			er2[i] = er.get(i).longValue();
		return er2;
	}
	
	/**
	 * Completes the given argument list argDest. For efficiency, arguments should be allocated
	 * directly as one list. So argDest is expected to already contain the non-keyword arguments.
	 */
	public static String[] prepareKeywordArgs(PyObject[] argsDest, PyDictionary keywords) {
		String[] er = new String[keywords.size()];
		int offset = argsDest.length-er.length;
		Iterator<Map.Entry<PyObject, PyObject>> kw = keywords.getMap().entrySet().iterator();
		Map.Entry<PyObject, PyObject> entry = kw.next();
		for (int i = 0; i < er.length; ++i) {
			er[i] = ((PyString) entry.getKey()).asString();
			argsDest[offset+i] = entry.getValue();
		}
		return er;
	}
	
	public static long getCurrentThreadID() {
		return Thread.currentThread().getId();
	}
	
	protected static int lastDictIndex;
	protected static PyDictionary lastDict;
	protected static Iterator<Map.Entry<PyObject, PyObject>> dictIterator;
	public static synchronized JyNIDictNextResult getPyDictionary_Next(PyDictionary dict, int index) {
		if (dict == lastDict && index == lastDictIndex && dictIterator != null) {
			Map.Entry<PyObject, PyObject> me;
			PyObject value;
			while (dictIterator.hasNext()) {
				++lastDictIndex;
				me = dictIterator.next();
				value = me.getValue();
				if (value != null) {
					if (lastDictIndex == dict.size()) lastDictIndex = -lastDictIndex;
					return new JyNIDictNextResult(lastDictIndex, me.getKey(), value);
				}
			}
		} else {
			lastDict = dict;
			dictIterator = dict.getMap().entrySet().iterator();
			lastDictIndex = 0;
			Map.Entry<PyObject, PyObject> me;
			while (lastDictIndex < index) {
				++lastDictIndex;
				if (!dictIterator.hasNext()) return null;
				else dictIterator.next();
			}
			PyObject value;
			while (dictIterator.hasNext()) {
				++lastDictIndex;
				me = dictIterator.next();
				value = me.getValue();
				if (value != null) {
					if (lastDictIndex == dict.size()) lastDictIndex = -lastDictIndex;
					return new JyNIDictNextResult(lastDictIndex, me.getKey(), value);
				}
			}
		}
		return null;
	}
	
	protected static int lastSetIndex;
	protected static BaseSet lastSet;
	protected static Iterator<PyObject> setIterator;
	public static synchronized JyNISetNextResult getPySet_Next(BaseSet set, int index) {
		if (set == lastSet && index == lastSetIndex && setIterator != null) {
			PyObject key;
			while (setIterator.hasNext()) {
				++lastSetIndex;
				key = setIterator.next();
				if (key != null) {
					if (lastSetIndex == set.size()) lastSetIndex = -lastSetIndex;
					return new JyNISetNextResult(lastSetIndex, key);
				}
			}
		} else {
			lastSet = set;
			try {
				Field backend = BaseSet.class.getDeclaredField("_set");
				backend.setAccessible(true);
				Set<PyObject> set2 = (Set<PyObject>) backend.get(set);
				setIterator = set2.iterator();
			} catch (Exception e) {
				lastSet = null;
				setIterator = null;
				return null;
			}
			lastSetIndex = 0;
			while (lastSetIndex < index) {
				++lastSetIndex;
				if (!setIterator.hasNext()) return null;
				else setIterator.next();
			}
			PyObject key;
			while (setIterator.hasNext()) {
				++lastSetIndex;
				key = setIterator.next();
				if (key != null) {
					if (lastSetIndex == set.size()) lastSetIndex = -lastSetIndex;
					return new JyNISetNextResult(lastSetIndex, key);
				}
			}
		}
		return null;
	}
	
	public static synchronized BaseSet copyPySet(BaseSet set) {
		if (set instanceof PySet)
			return new PySet(set);
		else if (set instanceof PyFrozenSet) return set;
		else return null;
	}
	
	public static void printPyLong(PyObject pl) {
		//System.out.println("printPyLong");
		System.out.println(((PyLong) pl).getValue());
	}
	
	/*public static JyNIDictNextResult getPyDictionary_Next(PyDictionary dict, int index)
	{
		if (index >= dict.size()) return null;
		Object[] ar = dict.entrySet().toArray();
		int nxt = index;
		Map.Entry me = (Map.Entry) ar[nxt++];
		while(me.getValue() == null)
		{
			if (nxt >= ar.length) return null;
			me = (Map.Entry) ar[nxt++];
		}
		if (nxt == ar.length) nxt = -1;
		return new JyNIDictNextResult(nxt, (PyObject) me.getKey(), (PyObject) me.getValue());
	}*/
	
	/*
	public static PyObject constructDefaultObject(Class pyObjectClass) throws IllegalArgumentException
	{
		if (pyObjectClass.equals(PyType.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyNotImplemented.class)) return Py.NotImplemented;
		else if (pyObjectClass.equals(PyNone.class)) return Py.None;
		else if (pyObjectClass.equals(PyFile.class)) return new PyFile();
		else if (pyObjectClass.equals(PyModule.class)) return new PyModule();
		else if (pyObjectClass.equals(PyCell.class)) return new PyCell();
		else if (pyObjectClass.equals(PyClass.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyInstance.class)) return new PyInstance();
		else if (pyObjectClass.equals(PyMethod.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyFunction.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyClassMethod.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyStaticMethod.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyMethodDescr.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyClassMethodDescr.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyDictProxy.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyProperty.class)) return new PyProperty();
		else if (pyObjectClass.equals(PyBoolean.class)) return new PyBoolean(false);
		else if (pyObjectClass.equals(PyFloat.class)) return new PyFloat(0.0f);
		else if (pyObjectClass.equals(PyInteger.class)) return new PyInteger(0);
		else if (pyObjectClass.equals(PyLong.class)) return new PyLong(0);
		else if (pyObjectClass.equals(PyComplex.class)) return new PyComplex(0, 0);
		else if (pyObjectClass.equals(PyUnicode.class)) return new PyUnicode();
		else if (pyObjectClass.equals(PyString.class)) return new PyString();
		else if (pyObjectClass.equals(PyBaseString.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" is considered an abstract class.");
		else if (pyObjectClass.equals(PySequenceIter.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyXRange.class)) return new PyXRange(0);
		else if (pyObjectClass.equals(PyTuple.class)) return new PyTuple();
		else if (pyObjectClass.equals(PyFastSequenceIter.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyList.class)) return new PyList();
		else if (pyObjectClass.equals(PyReversedIterator.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyDictionary.class)) return new PyDictionary();
		else if (pyObjectClass.equals(PySet.class)) return new PySet();
		else if (pyObjectClass.equals(PyFrozenSet.class)) return new PyFrozenSet();
		else if (pyObjectClass.equals(PyEnumerate.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PySlice.class)) return new PySlice();
		else if (pyObjectClass.equals(PyEllipsis.class)) return Py.Ellipsis;
		else if (pyObjectClass.equals(PyGenerator.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyCode.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyCallIter.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyFrame.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PySuper.class)) return new PySuper();
		else if (pyObjectClass.equals(PyBaseException.class)) return new PyBaseException();
		else if (pyObjectClass.equals(PyTraceback.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" cannot be constructed this way.");
		else if (pyObjectClass.equals(PyByteArray.class)) return new PyByteArray();
		else if (pyObjectClass.equals(PyObject.class)) throw new IllegalArgumentException(pyObjectClass.getName()+" is considered an abstract class.");
		else throw new IllegalArgumentException("Given class refers to no PyObject known by JNI");
	}*/
	
	/*public static PyObject callModuleFunction(CPythonModule module, String name, PyObject self, PyObject... args)
	{
		JyObject[] jyArgs = new JyObject[args.length];
		for (int i = 0; i < jyArgs.length; ++i)
		{
			jyArgs[i] = jyObjects.get(args[i]);
			if (jyArgs[i] == null)
			{
				jyArgs[i] = new JyObject(args[i]);
				jyObjects.put(args[i], jyArgs[i]);
			}
		}
		if (self != null)
		{
			JyObject self2 = jyObjects.get(self);
			if (self2 == null)
			{
				self2 = new JyObject(self);
				jyObjects.put(self, self2);
			}
			JyObject er = callModuleFunction(module, name, self2, jyArgs);
			jyObjects.put(er.object, er);
			return er.object;
		} else
		{
			JyObject er = callModuleFunction(module, name, null, jyArgs);
			jyObjects.put(er.object, er);
			return er.object;
		}
	}*/

	public static long[] lookupNativeHandles(PyList lst) {
		PyObject[] obj = lst.getArray();
		long[] er = new long[obj.length];
		for (int i = 0; i < er.length; ++i)
			er[i] = lookupNativeHandle(obj[i]);
		return er;
	}

	public static void GCTrackPyCPeer(PyCPeer peer) {
		new JyWeakReferenceGC(peer);
	}

	public static PyObjectGCHead makeGCHead(long handle, boolean forMirror, boolean gc) {
		//Todo: Use a simpler head if object cannot have links.
		PyObjectGCHead result;
		if (gc) result = forMirror ? new CMirrorGCHead(handle) : new CStubGCHead(handle);
		else result = forMirror ? new CMirrorSimpleGCHead(handle) : new CStubSimpleGCHead(handle);
		new JyWeakReferenceGC(result);
		return result;
	}

	public static JyGCHead makeStaticGCHead(long handle, boolean gc) {
		return gc ? new DefaultTraversableGCHead(handle) : new SimpleGCHead(handle);
	}

	//--------------errors-section-----------------
	public static PyObject exceptionByName(String name) {
//		System.out.println("look for exception: "+name);
		String rawName = name;
		int pin = name.indexOf('.');
		if (pin != -1) rawName = rawName.substring(pin+1);
//		System.out.println("rawName: "+rawName);
		try {
			Field exc = Py.class.getField(rawName);
			PyObject er = (PyObject) exc.get(null);
//			System.out.println("return "+er);
//			System.out.println("class: "+er.getClass());
			return er;
		} catch (Exception e) {
			System.err.println("JyNI-Warning: Could not obtain Exception: "+name);
			System.err.println("  Reason: "+e);
			return null;
		}
	}

	/*public static void JyErr_SetCurExc(ThreadState tstate, PyObject type, PyObject value, PyTraceback traceback)
	{
		ThreadState tstate0 = tstate == null ? Py.getThreadState() : tstate;
		PyException curexc = cur_excLookup.get(tstate0);
		if (curexc == null)
		{
			curexc = new PyException(type, value, traceback);
			cur_excLookup.put(tstate0, curexc);
		} else
		{
			curexc.type = type;
			curexc.value = value;
			curexc.traceback = traceback;
		}
	}
	
	public static PyException JyErr_GetCurExc(ThreadState tstate)
	{
		ThreadState tstate0 = tstate == null ? Py.getThreadState() : tstate;
		return cur_excLookup.get(tstate0);
	}*/

	protected static PyObject maybeExc(PyObject obj) throws PyException {
		if (obj == null && Py.getThreadState().exception != null) throw Py.getThreadState().exception;
		else return obj;
	}

	public static void JyErr_InsertCurExc(ThreadState tstate, PyObject type, PyObject value, PyTraceback traceback) {
		if (type == null) type = Py.None;
		if (value == null) value = Py.None;
		ThreadState tstate0 = tstate == null ? Py.getThreadState() : tstate;
		tstate0.exception = new PyException(type, value, traceback);
//		PyException cur_exc = cur_excLookup.remove(tstate0);
//		if (cur_exc != null)
//		{
//			tstate0.exception = cur_exc;
//		}
		//System.out.println("JyErr_InsertCurExc 3");
	}

	public static void JyErr_InsertCurExc(ThreadState tstate, PyObject type, PyObject value) {
		if (type == null) type = Py.None;
		if (value == null) value = Py.None;
		ThreadState tstate0 = tstate == null ? Py.getThreadState() : tstate;
		tstate0.exception = new PyException(type, value);
	}

	public static void JyErr_InsertCurExc(ThreadState tstate, PyObject type) {
		if (type == null) type = Py.None;
		ThreadState tstate0 = tstate == null ? Py.getThreadState() : tstate;
		tstate0.exception = new PyException(type);
	}

	public static void JyErr_InsertCurExc(ThreadState tstate) {
		ThreadState tstate0 = tstate == null ? Py.getThreadState() : tstate;
		tstate0.exception = new PyException();
	}

	/*public static void PyErr_Restore(PyObject type, PyObject value, PyTraceback traceback) {
		//ThreadState tstate = ;
		//tstate.exception = traceback instanceof PyTraceback ? new PyException(type, value, (PyTraceback) traceback) : new PyException(type, value);
		JyErr_SetCurExc(Py.getThreadState(), type, value, traceback);
	}
	
	public static void PyErr_Clear() {
		//ThreadState tstate = Py.getThreadState();
		//tstate.exception = null;
		cur_excLookup.remove(Py.getThreadState());
	}
	
	public static PyException PyErr_Fetch() {
		//ThreadState tstate = Py.getThreadState();
		//PyException er = tstate.exception;
		PyException er = cur_excLookup.remove(Py.getThreadState());
		//PyErr_Clear();
		return er;
	}
	
	public static PyObject PyErr_Occurred() {
		PyException er = cur_excLookup.get(Py.getThreadState());
		return er == null ? null : er.type;
//		ThreadState tstate = Py.getThreadState();
//		if (tstate.exception != null)
//		{
//			System.out.println("PyErr_Occurred: "+tstate.exception.getMessage());
//			System.out.println(tstate.exception);
//			System.out.println("value "+tstate.exception.value);
//			System.out.println("type "+tstate.exception.type);
//		}
//		return tstate.exception == null ? null : tstate.exception.type;
	}*/

	public static boolean PyErr_ExceptionMatches(PyObject exc, PyObject type, PyObject value, PyTraceback traceback) {
		if (type == null) type = Py.None;
		if (value == null) value = Py.None;
		//PyException cur_exc = cur_excLookup.get(Py.getThreadState());
		//return cur_exc == null ? exc == null : cur_exc.match(exc);
		return (new PyException(type, value, traceback)).match(exc);
//		ThreadState tstate = Py.getThreadState();
//		return tstate.exception == null ? exc == null : tstate.exception.match(exc);
	}

	/*public static void PyErr_SetObject(PyObject exception, PyObject value) {
		PyErr_Restore(exception, value, null);
	}
	
	public static void PyErr_SetString(PyObject exc, String value) {
		PyErr_Restore(exc, Py.newString(value), null);
	}
	
	public static void PyErr_SetNone(PyObject exc) {
		PyErr_Restore(exc, null, null);
	}
	
	public static PyObject PyErr_NoMemory() {
		if (PyErr_ExceptionMatches(Py.MemoryError))
			// already current
			return null;

		// raise the pre-allocated instance if it still exists
//		if (PyExc_MemoryErrorInst)
//			PyErr_SetObject(PyExc_MemoryError, PyExc_MemoryErrorInst);
//		else
//			// this will probably fail since there's no memory and hee,
//			// hee, we have to instantiate this class

		PyErr_SetNone(Py.MemoryError);

		return null;
	}*/

	public static void PyErr_WriteUnraisable(PyObject obj) {
		//Todo: Create and use something like JyNIUnraisableError instead of UnknownError.
		Py.writeUnraisable(new UnknownError("JyNI caused unraisable exception"), obj);
		
		//System.err.println("WriteUnraisableException:");
		//System.err.println(obj);
		//System.err.println(PyErr_Fetch());
		/*PyObject *f, *t, *v, *tb;
		PyErr_Fetch(&t, &v, &tb);
		f = PySys_GetObject("stderr");
		if (f != NULL) {
			PyFile_WriteString("Exception ", f);
			if (t) {
				PyObject* moduleName;
				char* className;
				assert(PyExceptionClass_Check(t));
				className = PyExceptionClass_Name(t);
				if (className != NULL) {
					char *dot = strrchr(className, '.');
					if (dot != NULL)
						className = dot+1;
				}

				moduleName = PyObject_GetAttrString(t, "__module__");
				if (moduleName == NULL)
					PyFile_WriteString("<unknown>", f);
				else {
					char* modstr = PyString_AsString(moduleName);
					if (modstr &&
						strcmp(modstr, "exceptions") != 0)
					{
						PyFile_WriteString(modstr, f);
						PyFile_WriteString(".", f);
					}
				}
				if (className == NULL)
					PyFile_WriteString("<unknown>", f);
				else
					PyFile_WriteString(className, f);
				if (v && v != Py_None) {
					PyFile_WriteString(": ", f);
					PyFile_WriteObject(v, f, 0);
				}
				Py_XDECREF(moduleName);
			}
			PyFile_WriteString(" in ", f);
			PyFile_WriteObject(obj, f, 0);
			PyFile_WriteString(" ignored\n", f);
			PyErr_Clear(); // Just in case
		}
		Py_XDECREF(t);
		Py_XDECREF(v);
		Py_XDECREF(tb);*/
	}

	/*public static void main(String[] args)
	{
		System.out.println(System.getProperty("user.dir"));
	}
	/*
		Class c = PyBoolean.class;
		try {
			Field tp = c.getField("TYPE");
			PyType t = (PyType) tp.get(null);
			System.out.println(t.getName());
		} catch (Exception e) {}
	}*/

	public static int slice_compare(PySlice v, PySlice w) {
		int result = 0;

		//if (v == w)
		if (v.equals(w))
			return 0;

		//if (PyObject_Cmp(v->start, w->start, &result) < 0)
		result = v.start.__cmp__(w.start);
		if (result < 0)
			return -2;
		if (result != 0)
			return result;
		//if (PyObject_Cmp(v->stop, w->stop, &result) < 0)
		result = v.stop.__cmp__(w.stop);
		if (result < 0)
			return -2;
		if (result != 0)
			return result;
		//if (PyObject_Cmp(v->step, w->step, &result) < 0)
		result = v.step.__cmp__(w.step);
		if (result < 0)
			return -2;
		return result;
	}

	public static String JyNI_pyCode_co_code(PyBaseCode code) {
		if (code instanceof PyBytecode) return new String(((PyBytecode) code).co_code);
		else if (code instanceof PyTableCode) return ((PyTableCode) code).co_code;
		else return null;
	}

	public static int JyNI_pyCode_co_flags(PyBaseCode code) {
		return code.co_flags.toBits();
	}

	public static String JyNI_pyCode_co_lnotab(PyBytecode code) {
		return new String(code.co_lnotab);
	}

	public static String getPlatform() {
		return PySystemState.version.asString();
	}

	public static void jPrint(String msg) {
		System.out.println(msg);
	}
	
	public static void jPrint(long val) {
		System.out.println(val);
	}

	public static void jPrintHash(Object val) {
		try {
			System.out.println(val.hashCode()+" ("+System.identityHashCode(val)+")");
		} catch (Exception e) {
			System.out.println("("+System.identityHashCode(val)+")");
		}
	}

	public static void jPrintInfo(Object val) {
		System.out.println("Object: "+val);
		System.out.println("Class: "+val.getClass());
	}

//---------gc-section-----------
	/*
	 * Stuff here is actually no public API, but partly
	 * declared public for interaction with JyNI.gc-package.
	 * Todo: Maybe move it to JyNI.-package and make it
	 *       protected or package-scoped.
	 */
	public static final int JYNI_GC_CONFIRMED_FLAG = 1;
	public static final int JYNI_GC_RESURRECTION_FLAG = 2;
	public static final int JYNI_GC_LAST_CONFIRMATION_FLAG = 4;
	//public static final int JYNI_GC_HANDLE_NO_CONFIRMATIONS = -1;

	//Confirm deletion of C-Stubs:
	static long[] confirmedDeletions, resurrections;
	static int confirmationsUnconsumed = 0;
	
	private static void gcDeletionReport(long[] confirmed, long[] resurrected) {
		boolean postProcess = false;

		//some debug-info:
		//System.out.println("gcDeletionReport");
//		if (confirmed != null) {
//			System.out.println("confirmed cstub deletions:");
//			for (int i = 0; i < confirmed.length; ++i) {
//				System.out.println("  "+confirmed[i]);
//			}
//		} else System.out.println("no confirmed cstub deletions");
//		if (resurrected != null) {
//			System.out.println("resurrections:");
//			for (int i = 0; i < resurrected.length; ++i) {
//				System.out.println("  "+resurrected[i]);
//			}
//		} else System.out.println("no cstub resurretions");
		
		synchronized (CStubGCHead.class) {
			if (confirmationsUnconsumed > 0) {
				/*
				 * This might happen if gc-runs overlap (i.e. a gc-run overlaps the
				 * finalization-phase of the previous one - depending on implementation
				 * details this can happen or not), which should not (cannot?) happen in
				 * usual operation, but maybe one can provoke it by calling System.gc in
				 * high frequency. Just in case - maybe we just wait a bit if this
				 * occurs (but release the monitor to let CStub-finalizers catch up).
				 * Or it is due to a JyNI-bug. Both cases should be investigated, so we
				 * notify the user:
				 */
				System.err.println("JyNI-warning: There are unconsumed gc-confirmations!");
				while (confirmationsUnconsumed > 0) {
					try {
						CStubGCHead.class.wait();
					} catch (InterruptedException ie) {}
				}
			}
			confirmedDeletions = confirmed;
			resurrections = resurrected;
			confirmationsUnconsumed =
					(confirmedDeletions == null ? 0 : confirmedDeletions.length) +
					(resurrections == null ? 0 : resurrections.length);
			if (confirmationsUnconsumed == 0)
				postProcess = true;
			else
				CStubGCHead.class.notify();
		}
		//System.out.println("unconsumed: "+confirmationsUnconsumed);
		if (postProcess) postProcessCStubGCCycle();
	}

	/**
	 * Do not call this method, it is internal API.
	 */
	public static int consumeConfirmation(long handle) {
		synchronized (CStubGCHead.class) {
			while (confirmationsUnconsumed == 0) {
				try {
					//System.out.println("consumeConfirmation waiting... "+handle);
					CStubGCHead.class.wait();
				} catch(InterruptedException ie) {}
			}
			//System.out.println("consumeConfirmation resume "+handle);
			int i;
			if (confirmedDeletions != null) {
				for (i = 0; i < confirmedDeletions.length; ++i) {
					if (confirmedDeletions[i] == handle) {
						int result = --confirmationsUnconsumed != 0 ?
								JYNI_GC_CONFIRMED_FLAG :
								(JYNI_GC_CONFIRMED_FLAG | JYNI_GC_LAST_CONFIRMATION_FLAG);
						//System.out.println("consumeConfirmation "+handle+" is deletion");
						CStubGCHead.class.notify();
						return result;
					}
				}
			}
			if (resurrections != null) {
				for (i = 0; i < resurrections.length; ++i) {
					if (resurrections[i] == handle) {
						CStubGCHead.class.notify();
						int result = --confirmationsUnconsumed != 0 ?
								(JYNI_GC_RESURRECTION_FLAG) :
								(JYNI_GC_RESURRECTION_FLAG | JYNI_GC_LAST_CONFIRMATION_FLAG);
						//System.out.println("consumeConfirmation "+handle+" is resurrection");
						CStubGCHead.class.notify();
						return result;
					}
				}
			}
			//System.out.println("consumeConfirmation "+handle+" not consumed anything");
			return 0;
		}
	}

	public static void waitForCStubs() {
		synchronized (CStubGCHead.class) {
			while (confirmationsUnconsumed > 0) {
				try {
					//System.out.println("waitForCStubs "+confirmationsUnconsumed);
					CStubGCHead.class.wait();
				} catch(InterruptedException ie) {}
			}
			//System.out.println("waitForCStubs done");
		}
	}

	protected static class visitRestoreCStubReachables implements Visitproc {
		static visitRestoreCStubReachables defaultInstance
				= new visitRestoreCStubReachables();

		Set<PyObject> alreadyExplored = new HashSet<>();
		public Stack<PyObject> explorationStack = new Stack<>();
		
		public static void clear() {
			defaultInstance.alreadyExplored.clear();
		}

		@Override
		public int visit(PyObject object, Object arg) {
			if (alreadyExplored.contains(object))
				return 0;
			if (continueCStubExplore(object)) {
				explorationStack.push(object);
			}
			alreadyExplored.add(object);
			CStubReachableRestore(object);
			return 0;
		}
	}

	protected static boolean continueCStubExplore(PyObject obj) {
		if (JyNICriticalObjectSet.contains(obj)) return false;
		if (!gc.isTraversable(obj)) return false;
		long handle = lookupNativeHandle(obj);
		JyWeakReferenceGC headRef = JyWeakReferenceGC.lookupJyGCHead(handle);
		if (headRef == null) return true;
		JyGCHead head = headRef.get();
		return head == null || !(head instanceof CStubGCHead || head instanceof CStubSimpleGCHead);
	}

	protected static void CStubReachableRestore(PyObject obj) {
		//System.out.println("Resotre: "+obj);
		gc.abortDelayedFinalization(obj);
		//gc.restoreFinalizer(obj); (done in abortDelayedFinalization)
		gc.restoreWeakReferences(obj);
	}

	/**
	 * Do not call this method, it is internal API.
	 */
	public static void CStubRestoreAllReachables(PyObject CStubBackend) {
		CStubReachableRestore(CStubBackend);
		if (gc.isTraversable(CStubBackend)) {
			gc.traverse(CStubBackend, visitRestoreCStubReachables.defaultInstance, null);
			while (!visitRestoreCStubReachables.defaultInstance.explorationStack.empty()) {
				gc.traverse(visitRestoreCStubReachables.defaultInstance.explorationStack.pop(),
						visitRestoreCStubReachables.defaultInstance, null);
			}
		}
	}

	/**
	 * Do not call this method, it is internal API.
	 */
	public static void postProcessCStubGCCycle() {
		/*
		 * Note: This method is *not* triggered by gc-module postFinalization process.
		 * Instead we know (do we?) from JyWeakReferenceGC how many CStub-finalizers
		 * are expected.
		 */
		visitRestoreCStubReachables.clear();
		//Should be done automatically:
		//GlobalRef.processDelayedCallbacks();
		//gc.notifyPostFinalization(); (maybe include this later)
	}

	/**
	 * Do not call this method, it is internal API.
	 */
	public static void preProcessCStubGCCycle() {
		/* We pretend to be another finalizer here ending in postProcessCStubGCCycle().
		 * We can do that, because we know when the last CStub finalizer is processed.
		 */
		//gc.notifyPreFinalization(); (maybe include this later)
		gc.removeJythonGCFlags(gc.FORCE_DELAYED_FINALIZATION);
		//Here we care to repair the referenceGraph and to restore weak references.
		//System.out.println("preProcessCStubGCCycle");
		JyWeakReferenceGC headRef;
		JyGCHead head;
		boolean delayFinalization = false;
		for (long handle: JyNICriticalObjectSet) {
			//System.out.println("  "+handle);
			headRef = JyWeakReferenceGC.lookupJyGCHead(handle);
			/*
			 * Evaluate JyGC_validateGCHead first, since it has the side-effect to
			 * update the JyNI-critical's GCHead if necessary. This is also the reason
			 * why we cannot break this loop on early success.
			 */
//			if (headRef == null) {
//				System.out.println("unexplored!");
//				//delayFinalization = JyGC_validateGCHead(handle, null) || delayFinalization;
//			} else {
			if (headRef == null) {
				head = headRef.get();
//				if (head == null) {
//					System.out.println("j-deleted!");
//				} else
				if (head != null && head instanceof TraversableGCHead) {
					delayFinalization = JyGC_validateGCHead(handle,
							((TraversableGCHead) head).toHandleArray()) || delayFinalization;
//					boolean tmp = JyGC_validateGCHead(handle,
//							((TraversableGCHead) head).toHandleArray());
//					delayFinalization = tmp || delayFinalization;
//					if (tmp)
//						System.out.println("check update repair: "+JyGC_validateGCHead(handle, ((TraversableGCHead) head).toHandleArray()));
				} else System.err.println(
						"JyNI-error: Encountered JyNI-critical with non-traversable JyGCHead!");
			}
		}
		if (delayFinalization) {//enable delayed finalization in GC-module
			//System.out.println("Force delayed finalization...");
			gc.addJythonGCFlags(gc.FORCE_DELAYED_FINALIZATION);
		}
	}

	protected static void suspendPyInstanceFinalizer(PyInstance inst) {
		FinalizeTrigger ft =
				   (FinalizeTrigger) JyAttribute.getAttr(inst, JyAttribute.FINALIZE_TRIGGER_ATTR);
		if (ft != null) ft.clear();
	}

	protected static void restorePyInstanceFinalizer(PyInstance inst) {
		FinalizeTrigger ft =
				   (FinalizeTrigger) JyAttribute.getAttr(inst, JyAttribute.FINALIZE_TRIGGER_ATTR);
		if (ft != null && (ft.flags & FinalizeTrigger.FINALIZED_FLAG) == 0) ft.trigger(inst);
		else gc.restoreFinalizer(inst);
	}
}
