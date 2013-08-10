package JyNI;

import java.lang.reflect.Field;
import java.util.List;
import java.util.Set;
import java.util.Collection;
import java.util.Iterator;
import org.python.core.BaseSet;

import org.python.core.PyObject;

public class JySet implements Set<PyObject> {
	protected Set<PyObject> backend;
	long nativeBackendHandle;
	
	public JySet(long nativeBackendHandle)
	{
		this.nativeBackendHandle = nativeBackendHandle;
	}
	
	public JySet(Set<PyObject> backend, long nativeBackendHandle)
	{
		this.backend = backend;
		this.nativeBackendHandle = nativeBackendHandle;
	}
	
	public void installToPySet(BaseSet set)
	{
		try
		{
			Field backend = BaseSet.class.getDeclaredField("_set");
			backend.setAccessible(true);
			Set<PyObject> er = (Set<PyObject>) backend.get(set);
			if (this.backend == null)
				this.backend = er;
			backend.set(set, this);
		} catch (Exception e)
		{
			System.err.println("Problem modifying PySet backend: "+e);
		}
	}
	
	public Set<PyObject> installToPySetAndGetOldBackend(BaseSet set)
	{
		try
		{
			Field backend = BaseSet.class.getDeclaredField("_set");
			backend.setAccessible(true);
			Set<PyObject> er = (Set<PyObject>) backend.get(set);
			if (this.backend == null)
				this.backend = er;
			backend.set(set, this);
			return er;
		} catch (Exception e)
		{
			System.err.println("Problem in installToPySetAndGetOldBackend: "+e);
			return null;
		}
	}
	
	public boolean add(PyObject e) {
		if (backend.add(e)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public boolean addAll(Collection<? extends PyObject> c) {
		if (backend.addAll(c)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public void clear() {
		backend.clear();
		JyNI.JySet_putSize(nativeBackendHandle, 0);
	}
	
	public boolean contains(Object o) {return backend.contains(o);}
	public boolean containsAll(Collection<?> c) {return backend.containsAll(c);}
	public boolean equals(Object o) {return backend.equals(o);}
	public int hashCode() {return backend.hashCode();}
	public boolean isEmpty() {return backend.isEmpty();}
	public Iterator<PyObject> iterator() {return backend.iterator();}
	
	public boolean remove(Object o) {
		if (backend.remove(o)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public boolean removeAll(Collection<?> c) {
		if (backend.removeAll(c)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public boolean retainAll(Collection<?> c) {
		if (backend.retainAll(c)) {
			JyNI.JySet_putSize(nativeBackendHandle, size());
			return true;
		} else return false;
	}
	
	public int size() {return backend.size();}
	public Object[] toArray() {return backend.toArray();}
	public <T> T[] toArray(T[] a) {return backend.toArray(a);}
}
