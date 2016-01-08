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

import org.python.core.PyObject;
import org.python.core.PyModule;
//import org.python.core.PyMethod;
import org.python.core.PySystemState;
import org.python.core.Py;
import org.python.core.imp;

public class TestTk {

	public static interface Tk {
		public void mainloop();
		public void destroy();
	}

	public static interface StringVar {
		public String get();
		public void set(String text);
	}

	public static interface Label {
		public void pack();
	}

	public static interface Button {
		public void pack();
	}

	public static interface Entry {
		public void pack();
	}


	static Tk root;
	static StringVar txt;

	public static void printText(PyObject[] args, String[] kws) {
		System.out.println(txt.get());
	}

	public static void printTimeStamp(PyObject[] args, String[] kws) {
		System.out.println("System.currentTimeMillis: "+System.currentTimeMillis());
	}

	public static void destroyRoot(PyObject[] args, String[] kws) {
		root.destroy();
	}

//	public static PyMethod makeMethod(Class cls, String name, Object self) {
//		return new PyMethod(Py.newJavaFunc(cls, name), Py.java2py(self), Py.java2py(self.getClass()));
//	}

	public static void main(String[] args) {

		PySystemState pystate = Py.getSystemState();
		pystate.path.add("/usr/lib/python2.7/lib-dynload");
		pystate.path.add("/usr/lib/python2.7/lib-tk");
		PyModule tkModule = (PyModule) imp.importName("Tkinter", true);

		root = tkModule.newJ(Tk.class);

		txt = tkModule.newJ(StringVar.class);
		txt.set("Hello World!");

		Label lab = tkModule.newJ(Label.class, new String[]{"text"},
				root, "Welcome to JyNI Tkinter-Demo!");
		lab.pack();

		Entry entry = tkModule.newJ(Entry.class, new String[]{"textvariable"}, root, txt);
		entry.pack();

		String[] kwTxtCmd = {"text", "command"};
		Button buttonPrint = tkModule.newJ(Button.class, kwTxtCmd,
				root, "print text", Py.newJavaFunc(TestTk.class, "printText"));
		buttonPrint.pack();

		Button buttonTimestamp = tkModule.newJ(Button.class, kwTxtCmd,
				root, "print timestamp", Py.newJavaFunc(TestTk.class, "printTimeStamp"));
//		For Java8 I want this to work:
//		Button button = Py.newJavaObject(tkModule, Button.class, keyWords1,
//				root, "print timestamp", TestTk::printTimeStamp);
		buttonTimestamp.pack();
		
		Button buttonQuit = tkModule.newJ(Button.class, kwTxtCmd,
				root, "Quit", //makeMethod(Tk.class, "destroy", root));
				Py.newJavaFunc(TestTk.class, "destroyRoot"));
		buttonQuit.pack();

		root.mainloop();

		System.out.println("====");
		System.out.println("exit");
		System.out.println("====");
	}
}
