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

import JyNI.gc.JyGCHead;
import JyNI.gc.JyWeakReferenceGC;
import JyNI.gc.JyNIFinalizeTriggerFactory;
import java.util.Properties;
import org.python.core.JythonInitializer;
import org.python.core.PySystemState;
import org.python.core.adapter.ExtensiblePyObjectAdapter;
import org.python.core.finalization.FinalizeTrigger;
import org.python.util.PythonInterpreter;
import org.python.modules.gc;
import org.python.modules._weakref.GlobalRef;

public class JyNIInitializer implements JythonInitializer {

	public static boolean initialized = false;

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
		//System.out.println("Init JyNI...");
		PySystemState initState = PySystemState.doInitialize(preProperties,
				postProperties, argv, classLoader, adapter);
		initState.path_hooks.append(new JyNIImporter());
//		initializeState(initState);
//	}
//
//	public void initializeState(PySystemState state) {
//		state.path_hooks.append(new JyNIImporter());
//		initializeBasic();
//	}
//
//	private void initializeBasic() {
		//add the JyNI-Importer to list of import hooks:
		//if (initialized) return;

		PythonInterpreter pint = new PythonInterpreter();
		//add support for sys.setdlopenflags and sys.getdlopenflags as available in common CPython:
		pint.exec("import sys");
		//pint.exec("import JyNI.JyNI");
		pint.exec("sys.dlopenflags = "+JyNI.RTLD_JyNI_DEFAULT);
		//pint.exec("sys.setdlopenflags = JyNI.JyNI.setDLOpenFlags");
		//pint.exec("sys.getdlopenflags = JyNI.JyNI.getDLOpenFlags");
		//pint.exec("sys.setdlopenflags = lambda n: (sys.dlopenflags = n)");
		pint.exec("def setdlopenflags(n): sys.dlopenflags = n");
		pint.exec("sys.setdlopenflags = setdlopenflags");
		pint.exec("sys.getdlopenflags = lambda: sys.dlopenflags");
		pint.cleanup();

		//Set up Jython hooks for JyNI:
		FinalizeTrigger.factory = new JyNIFinalizeTriggerFactory();
		//GlobalRef.factory = new JyNIGlobalRefFactory();
		new SentinelFinalizer();
		gc.addJythonGCFlags(gc.FORCE_DELAYED_WEAKREF_CALLBACKS);
		gc.registerPreFinalizationProcess(new Runnable(){
				public void run() {JyNI.preProcessCStubGCCycle();}});
		//System.out.println("Init JyNI done");
		initialized = true;
	}
}
