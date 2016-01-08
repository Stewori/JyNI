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
//import java.util.Map.Entry;

public class JyNIDictNextResult {
	public PyObject key, value;
	public int newIndex;
	public long keyHandle, valueHandle;
	
	public JyNIDictNextResult(int newIndex, PyObject key, PyObject value)
	{
		this.newIndex = newIndex;
		this.key = key;
		this.value = value;
		keyHandle = JyNI.lookupNativeHandle(key);
		valueHandle = JyNI.lookupNativeHandle(value);
	}
	
	public JyNIDictNextResult(int newIndex, PyObject key, PyObject value, long keyHandle, long valueHandle)
	{
		this.newIndex = newIndex;
		this.key = key;
		this.value = value;
		this.keyHandle = keyHandle;
		this.valueHandle = valueHandle;
	}
}
