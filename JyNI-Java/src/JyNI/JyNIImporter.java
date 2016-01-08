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
import org.python.core.Untraversable;
import java.io.File;
import java.util.Arrays;
import java.util.Vector;
import java.util.List;
import java.util.HashMap;

/**
 * Load native modules.
 */
@Untraversable
public class JyNIImporter extends PyObject {
	public static HashMap<String, JyNIModuleInfo> dynModules = new HashMap<String, JyNIModuleInfo>();
	
	List<String> knownPaths = null;
	Vector<String> libPaths = new Vector<String>();

	public JyNIImporter() {
		super();
	}
	
	public JyNIImporter(List<String> knownPaths) {
		super();
		this.knownPaths = knownPaths;
	}
	
	public JyNIImporter(String... knownPaths) {
		super();
		this.knownPaths = Arrays.asList(knownPaths);
	}
	
	public PyObject __call__(PyObject args[], String keywords[]) {
		String s = args[0].toString();
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
		/*Py.writeDebug("import", "trying " + name
				+ " in packagemanager for path " + path);
		PyObject ret = PySystemState.packageManager.lookupName(name.intern());
		if (ret != null) {
			Py.writeComment("import", "'" + name + "' as java package");
			return this;
		}*/
		String suf = "."+getSystemDependendDynamicLibraryExtension();
		for (String s : libPaths)
		{
			File fl = new File(s);
			//if (fl.exists())
			String[] ch = fl.list();
			if (ch != null)
			{
				for (String m : ch)
				{
					//System.out.println("Check: "+m);
					//if (m.equals(name+suf))
					if (m.startsWith(name+".") && m.endsWith(suf))
					{
						//System.out.println("CPythonExtensionImporter found extension "+name);
						//System.out.println("Extension-Path: "+s+File.separatorChar+m);
						dynModules.put(name, new JyNIModuleInfo(name, s+File.separatorChar+m, null));
						return this;
					}
				}	
			}
		}
		//System.out.println("CPythonExtensionImporter8: "+name);
		//System.out.println("Path: "+path);
		//System.out.println("look in "+libPath);
		return Py.None;
	}

	public PyObject load_module(String name) {
		//ToDo:
		//Maybe check via 
		//imp.loadBuiltin and
		//imp.loadFromSource if
		//name is maybe just some Python script with an odd name.
		//CPython also works as usual if an ordinary script is called foo.so.
		//System.out.println("JyNIImporter.load module: "+name);
		JyNIModuleInfo inf = dynModules.get(name);
		if (inf.module == null)
		{
			inf.module = JyNI.loadModule(name, inf.path, JyTState.prepareNativeThreadState(Py.getThreadState()));
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
	
	public static String getSystemDependendDynamicLibraryExtension() {
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