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


package JyNI;

import org.python.core.Py;
import org.python.core.PyModule;
import org.python.core.PyObject;
import org.python.core.PySystemState;
import org.python.core.Untraversable;
import org.python.core.imp;

import java.io.File;
import java.util.Vector;
import java.util.List;
import java.util.Arrays;
import java.util.HashMap;

/**
 * Load native modules.
 */
@Untraversable
public class JyNIImporter extends PyObject {
	public static HashMap<String, JyNIModuleInfo> dynModules = new HashMap<String, JyNIModuleInfo>();

	/* Note that e.g. datetime is not on the blacklist because native version works well with JyNI
	 * and is sometimes required (e.g. by numpy) because it features a C-API.
	 * It appears that Java-implemented modules are always preferred. However collisions can occur
	 * if Jython features a C-extension replacement that is implemented as a .py file, at least on
	 * top-level. So for now we include only these into the blacklist, but keep the other potential
	 * collisions prepared as comments.
	 */
	public static List<String> blacklist = Arrays.asList(
			"crypt",           // JyNI can also import native variant
			"future_builtins", // also works with JyNI to some extend (numpy)
			"grp",             // fails with JyNI
			"_io",             // fails with JyNI
			"pyexpat",         // JyNI can also import native variant
			"select",          // JyNI can also import native variant
			"_socket",         // JyNI can also import native variant
			"unicodedata",     // JyNI can also import native variant
			"zlib",            // JyNI can also import native variant
			"readline"

// Java-implemented modules:
//			"array", "binascii", "cmath", "_collections", "cPickle",
//			"cStringIO", "_csv", "_functools", "itertools", "_json",
//			"math", "operator", "_random", "time", "bz2",
			);

	List<String> knownPaths = null;
	Vector<String> libPaths = new Vector<String>();

	public JyNIImporter() {
		super();
	}

// Currently not used:
//	public JyNIImporter(List<String> knownPaths) {
//		super();
//		this.knownPaths = knownPaths;
//		PyModule osModule = (PyModule) imp.importName("os", true);
//		osModule.__setattr__("name".intern(), new PyOSNameString());
//	}
//	
//	public JyNIImporter(String... knownPaths) {
//		super();
//		this.knownPaths = Arrays.asList(knownPaths);
//		PyModule osModule = (PyModule) imp.importName("os", true);
//		osModule.__setattr__("name".intern(), new PyOSNameString());
//	}

	public PyObject __call__(PyObject args[], String keywords[]) {
		String s = args[0].toString();
//		System.out.print("JyNI call... "+s);

		/*
		 * In order to be recognized by Jython we must return 'this' at
		 * least once. For statically linked modules no corresponding
		 * paths are requested here. On the other hand always returning
		 * 'this' would  break ordinary situations, so we return positive
		 * for the JyNI Lib-folder, which is always requested, even if it
		 * does not exist.
		 */
		if (s.endsWith("Lib")) return this;

		File f = new File(s);
		if (!f.exists() || !f.isDirectory()) {
			throw Py.ImportError("unable to handle");
		}
		if (knownPaths == null)
		{
			if (!libPaths.contains(s)) libPaths.add(s);
			return this;
		} else if (knownPaths.contains(s))
		{
			if (!libPaths.contains(s)) libPaths.add(s);
			return this;
		}
		throw Py.ImportError("unable to handle");
	}

	/**
	 * Find the module for the fully qualified name.
	 *
	 * @param name the fully qualified name of the module
	 * @return a loader instance if this importer can load the module, None
	 *		 otherwise
	 */
	public PyObject find_module(String name) {
		return find_module(name, Py.None);
	}

	/**
	 * Find the module for the fully qualified name.
	 *
	 * @param name the fully qualified name of the module
	 * @param path if installed on the meta-path None or a module path
	 * @return a loader instance if this importer can load the module, None
	 *		 otherwise
	 */
	public PyObject find_module(String name, PyObject path) {
//		System.out.print("JyNI find... "+name);
		if (dynModules.containsKey(name)) {/*System.out.println(" JyNI cache");*/ return this;}
		/*Py.writeDebug("import", "trying " + name
				+ " in packagemanager for path " + path);
		PyObject ret = PySystemState.packageManager.lookupName(name.intern());
		if (ret != null) {
			Py.writeComment("import", "'" + name + "' as java package");
			return this;
		}*/
		String subPath;
		String modname;
		int lastDot = name.lastIndexOf('.');
		if (lastDot == -1) {
			subPath = null;
			modname = name;
		} else {
			subPath = name.substring(0, lastDot);
			modname = name.substring(lastDot+1);
			subPath = subPath.replace('.', File.separatorChar);
		}
		if (blacklist.contains(modname)) return Py.None;
		String suf = "."+getSystemDependentDynamicLibraryExtension();
		for (String s : libPaths)
		{
//			System.out.println("JyNI look in "+s+File.separatorChar+subPath);
			if (subPath != null) s = s+File.separatorChar+subPath;
			File fl = new File(s);
			//if (fl.exists())
			String[] ch = fl.list();
			if (ch != null)
			{
				for (String m : ch)
				{
					//System.out.println("Check: "+m);
					//if (m.equals(name+suf))
					if (m.startsWith(modname+".") && m.endsWith(suf))
					{
						//System.out.println("CPythonExtensionImporter found extension "+name);
						//System.out.println("Extension-Path: "+s+File.separatorChar+m);
						dynModules.put(name, new JyNIModuleInfo(name, s+File.separatorChar+m, null));
//						System.out.println(" JyNI ok");
						return this;
					}
				}	
			}
		}
		//System.out.println("CPythonExtensionImporter8: "+name);
		//System.out.println("Path: "+path);
		//System.out.println("look in "+libPath);
//		System.out.println(" can't handle");
		return Py.None;
	}

	public PyObject load_module(String name) {
//		System.out.println("JyNI load... "+name);
		// This stuff should move to JyNIInitializer, but there it currently
		// breaks sysconfig.py. Is fixed in Jython 2.7.1 final.
		PySystemState sysState = Py.getSystemState();
		if (!(sysState.getPlatform() instanceof PyShadowString)) {
			sysState.setPlatform(new PyShadowString(sysState.getPlatform(),
					JyNI.getNativePlatform()));
			PyModule osModule = (PyModule) imp.importName("os", true);
			String _name = osModule.__getattr__("_name".intern()).toString();
			String nameval = "name".intern();
			PyObject osname = osModule.__getattr__(nameval);
			osModule.__setattr__(nameval, new PyShadowString(osname, _name));
		}

		//ToDo:
		//Maybe check via 
		//imp.loadBuiltin and
		//imp.loadFromSource if
		//name is maybe just some Python script with an odd name.
		//CPython also works as usual if an ordinary script is called foo.so.
		//System.out.println("JyNIImporter.load module: "+name);
		JyNIModuleInfo inf = dynModules.get(name);
//		if (inf == null) {
//			System.out.println("inf null: "+name);
//			for (String key: dynModules.keySet())
//				System.out.println(key);
//		}
		if (inf.module == null)
		{
			inf.module = JyNI.loadModule(name, inf.path, JyTState.prepareNativeThreadState(Py.getThreadState()));
			JyNI.maybeExc(inf.module);
			//System.out.println("had to call JyNI.loadModule, which returned "+inf.module);
		}
		//return JyNI.loadModule(name, "path");
		//System.out.println("JyNIImporter.load_module returns: "+inf.module);
		return inf.module;
		//System.out.println("000path: "+inf.path);
		//return Py.NotImplemented; //PySystemState.packageManager.lookupName(name.intern());
	}

	/**
	 * Returns a string representation of the object.
	 *
	 * @return a string representation of the object.
	 */
	public String toString() {
		return this.getType().toString();
	}

	public static String getSystemDependentDynamicLibraryExtension() {
		String OS = System.getProperty("os.name").toLowerCase();
		//if isWindows:
		if (OS.indexOf("win") >= 0) return "dll";
		else return "so";
	}

	/**	This method is actually not needed, since CPython-extensions ignore this naming-standard.
		Filenames only differ in the ending, i.e. ".so" vs ".dll".
	*/
	public static String libNameToFileName(String libName) {
		String OS = System.getProperty("os.name").toLowerCase();
		//isWindows:
		if (OS.indexOf("win") >= 0) return libName+".dll";
		else return "lib"+libName+".so";
		/*
		//isUnix:
		if (OS.indexOf("nix") >= 0 || OS.indexOf("nux") >= 0 || OS.indexOf("aix") > 0 ) return "lib"+libName+".so";
		//isMac:
		if (OS.indexOf("mac") >= 0) return "lib"+libName+".so";
		//isSolaris:
		if (OS.indexOf("sunos") >= 0) return "lib"+libName+".so";*/
	}
}
