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

import JyNI.gc.JyGCHead;
import JyNI.gc.JyWeakReferenceGC;
import JyNI.gc.JyNIFinalizeTriggerFactory;

import java.io.File;
import java.util.Properties;
import java.security.CodeSource;
import org.python.core.JythonInitializer;
import org.python.core.PySystemState;
import org.python.core.Py;
import org.python.core.imp;
import org.python.core.PyModule;
import org.python.core.PyShadowString;
import org.python.core.adapter.ExtensiblePyObjectAdapter;
import org.python.core.finalization.FinalizeTrigger;
import org.python.modules.gc;
import org.python.modules._weakref.GlobalRef;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class JyNIInitializer implements JythonInitializer {

	protected static boolean initialized = false;
	protected static boolean isWindows = System.getProperty("os.name").startsWith("Windows");
	static JyNIImporter importer;

	static class SentinelFinalizer implements JyGCHead {
		public SentinelFinalizer() {
			new JyWeakReferenceGC(this);
		}

		protected void finalize() throws Throwable {
			gc.notifyPreFinalization();
			new SentinelFinalizer();
			gc.notifyPostFinalization();
		}

		public long getHandle() {return 0;}
	}

	public void initialize(Properties preProperties, Properties postProperties, String[] argv,
			ClassLoader classLoader, ExtensiblePyObjectAdapter adapter)
	{
//		System.out.println("Init JyNI...");
//		System.out.println(System.getProperty("java.class.path"));
		//System.getProperties().list(System.out);
		PySystemState initState = PySystemState.doInitialize(preProperties,
				postProperties, argv, classLoader, adapter);
		importer = new JyNIImporter();
		// Add libs statically linked into JyNI:
		// (Using null-path causes lookup in main executable)
		//JyNIImporter.dynModules.put("_tkinter", new JyNIModuleInfo("_tkinter", null, null));

		initState.path_hooks.append(importer);

		PyModule osModule = (PyModule) imp.importName("os", true);
		PyShadowString os_name = ((PyShadowString) osModule.__getattr__("name".intern()));

		// We make sure that JyNI.jar is not only on classpath, but also on Jython-path:
		try
		{
			Class mClass = JyNIInitializer.class;
			CodeSource mCodeSource = mClass.getProtectionDomain().getCodeSource();
			initState.path.add(0, (new File(mCodeSource.getLocation().toURI()).getAbsolutePath()));
		}
		catch (Exception ex)
		{
			String[] cp = System.getProperty("java.class.path").split(File.pathSeparator);
			for (int i = 0; i < cp.length; ++i) {
				/*
					\b WordBoundary				
					. All characters 
					* 0 or n times
					Need to start by JyNI and only JyNI (JyNII will return false, but JyNI-a will return true)
					Need to end by .jar
					the jar need to be named JyNI[...].jar
				*/
				Pattern p  = Pattern.compile("\\bJyNI\\b.*\\b\\.jar\\b");
				Matcher m = p.matcher(cp[i]);
				if (m.find()) {
					initState.path.add(0, cp[i]);
					//initState.path.add(0, cp[i]+"/lib-tk");
				}
			}
		}
		// We add some targets for receiving native os.name and sys.platform values:
		// (this list will need constant maintenance as extension support grows)
		os_name.addTarget("ctypes.*", null);
		os_name.addTarget("OpenGL.*", null);
		// Since we do it right after initialization, this will affect PySystemState.defaultPlatform
		// and thus all subsequently created PySystemStates too.
		// That's why we needn't do it in PySystemStateJyNI. 
		((PyShadowString) initState.platform).addTarget("OpenGL.*", null);
		((PyShadowString) initState.platform).addTarget("Tkinter.*", null);

		//add support for sys.setdlopenflags and sys.getdlopenflags as available in common CPython:
		initState.__setattr__(JyNI.DLOPENFLAGS_NAME, Py.newInteger(JyNI.RTLD_JyNI_DEFAULT));
		initState.__setattr__("setdlopenflags".intern(), Py.newJavaFunc(JyNI.class, "sys_setdlopenflags"));
		initState.__setattr__("getdlopenflags".intern(), Py.newJavaFunc(JyNI.class, "sys_getdlopenflags"));
		if (isWindows) {
			WindowsException.exceptionsAddWindowsError();
		}

		//Set up Jython hooks for JyNI:
		FinalizeTrigger.factory = new JyNIFinalizeTriggerFactory();
		GlobalRef.factory = new JyNIGlobalRefFactory();
		new SentinelFinalizer();
		gc.addJythonGCFlags(gc.FORCE_DELAYED_WEAKREF_CALLBACKS);
		gc.registerPreFinalizationProcess(new Runnable(){
				public void run() {JyNI.preProcessCStubGCCycle();}});

		/* Here we patch os.environ to use a proper putenv implementation.
		 * This gets it close to CPython semantics. */
		PyModule config_util = (PyModule) imp.load("config_util");
		config_util.__findattr__("_patch_os_environ").__call__();

		if (isWindows) {
			/* This enables support for mbcs encoding if we run on Windows. */
			config_util.__findattr__("_register_mbcs_encoding").__call__();
		}
//		System.out.println("Init JyNI done");
		initialized = true;
	}
}
