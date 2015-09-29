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

		root = Py.newJavaObject(tkModule, Tk.class);

		txt = Py.newJavaObject(tkModule, StringVar.class);
		txt.set("Hello World!");

		String[] keyWords0 = {"text"};
		Label lab = Py.newJavaObject(tkModule, Label.class,
				keyWords0, root, "Welcome to JyNI Tkinter-Demo!");
		lab.pack();

		String[] keyWords1 = {"textvariable"};
		Entry entry = Py.newJavaObject(tkModule, Entry.class, keyWords1, root, txt);
		entry.pack();

		String[] keyWords2 = {"text", "command"};
		Button buttonPrint = Py.newJavaObject(tkModule, Button.class, keyWords2,
				root, "print text", Py.newJavaFunc(TestTk.class, "printText"));
		buttonPrint.pack();

		Button buttonTimestamp = Py.newJavaObject(tkModule, Button.class,
				keyWords2, root, "print timestamp", Py.newJavaFunc(TestTk.class, "printTimeStamp"));
//		For Java8 I want this to work:
//		Button button = Py.newJavaObject(tkModule, Button.class, keyWords1,
//				root, "print timestamp", TestTk::printTimeStamp);
		buttonTimestamp.pack();
		
		Button buttonQuit = Py.newJavaObject(tkModule, Button.class,
				keyWords2, root, "Quit", //makeMethod(Tk.class, "destroy", root));
				Py.newJavaFunc(TestTk.class, "destroyRoot"));
		buttonQuit.pack();

		root.mainloop();

		System.out.println("====");
		System.out.println("exit");
		System.out.println("====");
	}
}
