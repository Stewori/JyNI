package JyNI;

import java.util.AbstractList;
import java.util.Collection;
import java.util.List;
import java.lang.reflect.Field;
import org.python.core.PyObject;
import org.python.core.PyList;

/**
 * JyList is an implementation of java.util.List, that
 * redirects all list-calls to a native backend, namely
 * a Python PyListObject as defined in listobject.h.
 * backendHandle must be a valid pointer to the backing
 * c-struct. The purpose of this implementation is to
 * allow perfect sync between a Jython org.python.core.PyList
 * object and a native PyListObject as defined in listobject.h.
 * Since the native object allows modification via macros, there
 * is no way to detect that modification and mirror it on java-side.
 * So the only way is, not to mirror anything, but to actually map
 * all list accesses to the original native data.
 * 
 * If JyNI syncs a native PyListObject to a jython PyList,
 * there are two cases. If the original object is native, a
 * Jython PyList is created with a JyList as backend.
 * If the original object is a Jython PyList, we must replace
 * its backend with a JyList. First we setup an appropriate
 * native PyListObject. Then we convert all objects from
 * the non-native PyList to native PyObjects and insert them
 * into the native list object. Having that done, we setup
 * a JyList with our new native PyListObject as backend and
 * finally replace the original backend of the PyList with
 * our newly created JyList. However since PyList's backend
 * is stored in a private final field, replacing it is a rather
 * sneaky operation, which indeed might corrupt Extensions written
 * in Java, that also customize backends of PyLists.
 * Future JyNI versions will provide configuration-options
 * to tune this behavior - for instance to sync the original
 * backend as far as possible, maybe even by polling the native
 * list for changes.
 * 
 * See http://docs.oracle.com/javase/specs/jls/se5.0/html/memory.html#17.5.3
 * for information about subsequently modifying final fields.
 * See also these discussions about setting private final fields:
 * stackoverflow.com/questions/3301635/change-private-static-final-field-using-java-reflection
 * stackoverflow.com/questions/4516381/changing-private-final-fields-via-reflection
 * Note that PyList sets its backend in the constructor, so
 * replacing it should work like mentioned in the second discussion.
 * (By the way, I am not involved in any of them, I just found them
 * and thought it would be useful here.)
 */
public class JyList extends AbstractList<PyObject> {
	long backendHandle;
	
	public JyList(long backendHandle) {
		super();
		this.backendHandle = backendHandle;
	}
	
	public JyList(Collection<PyObject> c, long backendHandle) {
		super();
		this.backendHandle = backendHandle;
		addAll(0, c);
	}
	
	public void installToPyList(PyList list)
	{
		try
		{
			Field backend = PyList.class.getDeclaredField("list");
			backend.setAccessible(true);
			backend.set(list, this);
		} catch (Exception e)
		{
			System.err.println("Problem modifying PyList backend: "+e);
		}
	}
	
	public List<PyObject> installToPyListAndGetOldBackend(PyList list)
	{
		try
		{
			Field backend = list.getClass().getDeclaredField("list");
			backend.setAccessible(true);
			List<PyObject> er = (List<PyObject>) backend.get(list);
			backend.set(list, this);
			return er;
		} catch (Exception e)
		{
			System.err.println("Problem in installToPyListAndGetOldBackend: "+e);
			return null;
		}
	}

	public PyObject get(int index) {
		return JyNI.JyList_get(backendHandle, index);
	}

	public int size() {
		
		return JyNI.JyList_size(backendHandle);
		
	}

	public PyObject set(int index, PyObject o) {
		return JyNI.JyList_set(backendHandle, index, o, JyNI.lookupNativeHandle(o));
	}
	
	public void add(int index, PyObject o) {
		JyNI.JyList_add(backendHandle, index, o, JyNI.lookupNativeHandle(o));
	}
	
	public PyObject remove(int index) {
		return JyNI.JyList_remove(backendHandle, index);
	}
}
