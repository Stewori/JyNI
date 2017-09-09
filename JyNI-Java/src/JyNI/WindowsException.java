package JyNI;

import org.python.core.Py;
import org.python.core.PyModule;
import org.python.core.PyObject;
import org.python.core.PyString;
import org.python.core.PyStringMap;
import org.python.core.PyTuple;
import org.python.core.PyType;
import org.python.core.exceptions;
import org.python.core.imp;
import java.lang.reflect.Field;

public abstract class WindowsException {
	public static PyObject WindowsError = null;

	protected static void exceptionsAddWindowsError() {
		String classname = "WindowsError".intern();
		WindowsError = buildClass(imp.load("exceptions").fastGetDict(), classname,
				"OSError", WindowsError(), "MS-Windows OS system call failed.");
		imp.load("__builtin__").__setattr__(classname, WindowsError);
	}

	/**
	 * Copied from org.python.core.exceptions.
	 */
	private static PyObject buildClass(PyObject dict, String classname, String superclass,
			PyObject classDict, String doc) {
		classDict.__setitem__("__doc__", Py.newString(doc));
		PyType type = (PyType)Py.makeClass("exceptions." + classname,
				dict.__finditem__(superclass), classDict);
		try
		{
			Field builtin = PyType.class.getDeclaredField("builtin");
			builtin.setAccessible(true);
			builtin.set(type, true);
		} catch (Exception e)
		{
			System.err.println("Problem setting builtin flag of WindowsError: "+e);
		}
		dict.__setitem__(classname, type);
		return type;
	}

	/**
	 * Copied from org.python.core.exceptions.
	 */
	private static void defineSlots(PyObject dict, String... slotNames) {
		PyObject[] slots = new PyObject[slotNames.length];
		for (int i = 0; i < slotNames.length; i++) {
			slots[i] = Py.newString(slotNames[i]);
		}
		dict.__setitem__("__slots__", new PyTuple(slots));
	}

	public static PyObject WindowsError() {
		PyObject dict = new PyStringMap();
		defineSlots(dict, "errno", "strerror", "filename", "winerror");
		/* Doc not yet covered:
		errno:	"POSIX exception code"
		strerror: "exception strerror"
		filename: "exception filename"
		winerror: "Win32 exception code"
		*/
		//dict.__setitem__("__init__", bindStaticJavaMethod("__init__", "WindowsError__init__"));
		dict.__setitem__("__str__", exceptions.bindStaticJavaMethod("__str__",
				WindowsException.class, "WindowsError__str__"));
		//dict.__setitem__("__reduce__", bindStaticJavaMethod("__reduce__", "EnvironmentError__reduce__"));
		return dict;
	}

	public static void WindowsError__init__(PyObject self, PyObject[] args, String[] kwargs) {
		exceptions.EnvironmentError__init__(self, args, kwargs);
		PyObject myerrno = self.__findattr__("errno");
		if (!myerrno.__nonzero__()) {
			return;
		}
		int errcode = myerrno.asInt();
		if (errcode == -1) {
			return;
		}
		int posix_errno = winerror_to_errno(errcode);
		self.__setattr__("errno", Py.newInteger(posix_errno));
		self.__setattr__("winerror", myerrno);
	}

	public static PyObject WindowsError__str__(PyObject self, PyObject[] args,
			String[] kwargs) {
		PyObject winerror = self.__findattr__("winerror");
		PyObject strerror = self.__findattr__("strerror");
		PyObject filename = self.__findattr__("filename");
		PyString result;
		if (filename.__nonzero__()) {
			result = Py.newString("[Errno %s] %s: %s");
			result = (PyString) result.__mod__(new PyTuple(winerror, strerror,
					filename.__repr__()));
		} else if (winerror.__nonzero__() && strerror.__nonzero__()) {
			result = Py.newString("[Errno %s] %s");
			result = (PyString) result.__mod__(new PyTuple(winerror, strerror));
		} else {
			return exceptions.EnvironmentError__str__(self, args, kwargs);
		}
		return result;
	}

	/*
	 * Copied from errmap.h
	 */
	public static int winerror_to_errno(int winerror)
	{
		switch(winerror) {
			case 2: return 2;
			case 3: return 2;
			case 4: return 24;
			case 5: return 13;
			case 6: return 9;
			case 7: return 12;
			case 8: return 12;
			case 9: return 12;
			case 10: return 7;
			case 11: return 8;
			case 15: return 2;
			case 16: return 13;
			case 17: return 18;
			case 18: return 2;
			case 19: return 13;
			case 20: return 13;
			case 21: return 13;
			case 22: return 13;
			case 23: return 13;
			case 24: return 13;
			case 25: return 13;
			case 26: return 13;
			case 27: return 13;
			case 28: return 13;
			case 29: return 13;
			case 30: return 13;
			case 31: return 13;
			case 32: return 13;
			case 33: return 13;
			case 34: return 13;
			case 35: return 13;
			case 36: return 13;
			case 53: return 2;
			case 65: return 13;
			case 67: return 2;
			case 80: return 17;
			case 82: return 13;
			case 83: return 13;
			case 89: return 11;
			case 108: return 13;
			case 109: return 32;
			case 112: return 28;
			case 114: return 9;
			case 128: return 10;
			case 129: return 10;
			case 130: return 9;
			case 132: return 13;
			case 145: return 41;
			case 158: return 13;
			case 161: return 2;
			case 164: return 11;
			case 167: return 13;
			case 183: return 17;
			case 188: return 8;
			case 189: return 8;
			case 190: return 8;
			case 191: return 8;
			case 192: return 8;
			case 193: return 8;
			case 194: return 8;
			case 195: return 8;
			case 196: return 8;
			case 197: return 8;
			case 198: return 8;
			case 199: return 8;
			case 200: return 8;
			case 201: return 8;
			case 202: return 8;
			case 206: return 2;
			case 215: return 11;
			case 1816: return 12;
			default: return 22; //EINVAL;
		}
	}
}
