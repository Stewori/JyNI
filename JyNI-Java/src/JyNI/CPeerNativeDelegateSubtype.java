package JyNI;

import org.python.core.PyObject;
import org.python.core.PyString;

/**
 * All methods in this interface must be implemented by a direct call
 * to the corresponding method in super.
 *
 * @author Stefan Richthofer
 */
public interface CPeerNativeDelegateSubtype extends CPeerInterface {

	public PyObject super__call__(PyObject[] args, String[] keywords);
	public PyObject super__findattr_ex__(String name);
	public void super__setattr__(String name, PyObject value);
	public PyString super__str__();
	public PyString super__repr__();
	public PyObject super__finditem__(PyObject key);
	public void super__setitem__(PyObject key, PyObject value);
	public void super__delitem__(PyObject key);
	public int super__len__();
	public String super_toString();
}
