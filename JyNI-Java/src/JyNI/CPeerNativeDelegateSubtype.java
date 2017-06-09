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
