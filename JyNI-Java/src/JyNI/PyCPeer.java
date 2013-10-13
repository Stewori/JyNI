/*
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013 Python Software Foundation.  All rights reserved.
 * 
 * Copyright of JyNI:
 * Copyright (c) 2013 Stefan Richthofer.  All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */


package JyNI;

import org.python.core.PyType;
import org.python.core.PyObject;
import org.python.core.PyString;
import org.python.core.PyTuple;
import org.python.core.PyDictionary;
import org.python.core.Py;

import java.util.HashMap;

public class PyCPeer extends PyObject {
	
	public long objectHandle;//, refHandle;
	
	public PyCPeer(long objectHandle, PyType subtype)
	{
		super(subtype);
		this.objectHandle = objectHandle;
		//JyNI.CPeerHandles.put(objectHandle, this);
	}
	
	public PyObject __call__(PyObject[] args, String[] keywords)
	{
		//System.out.println("CPeer called...");
		//System.out.println("args: "+args);
		//System.out.println("arg count: "+args.length);
		//if (keywords !=)
		/*System.out.println("PeerCall kw: "+keywords.length);
		for(int i = 0; i < keywords.length; ++i)
			System.out.println(keywords[i]);
		System.out.println("PeerCall args: "+args.length);
		for(int i = 0; i < args.length; ++i)
			System.out.println(args[i]);*/
		//PyObject er =
		if (keywords.length == 0)
			return JyNI.callPyCPeer(objectHandle,
				args.length == 0 ? Py.EmptyTuple : new PyTuple(args, false), null);//Py.None);
		else {
			//todo: Use PyStringMap here... much work needs to be done to make the peer dictobject accept this
			HashMap<PyObject, PyObject> back = new HashMap<PyObject, PyObject>(keywords.length);
			for (int i = 0; i < keywords.length; ++i)
			{
				back.put(Py.newString(keywords[i]), args[args.length-keywords.length+i]);
			}
			
			if (args.length > keywords.length)
			{
				PyObject[] args2 = new PyObject[args.length - keywords.length];
				System.arraycopy(args, 0, args2, 0, args2.length);
				return JyNI.callPyCPeer(objectHandle, new PyTuple(args2, false), new PyDictionary(back));
			} else
				return JyNI.callPyCPeer(objectHandle, Py.EmptyTuple, new PyDictionary(back));
		}
		
		/*System.out.println("Call er:");
		System.out.println(er);
		System.out.println(er.getClass().getName());
		System.out.println(er.getType().getName());*/
		//return er;
	}

	public PyObject __findattr_ex__(String name)
	{
		//System.out.println("Look for attribute "+name+" in PyCPeer");
		PyObject er = JyNI.getAttrString(objectHandle, name);
		return er != null ? er : Py.None;
		//return super.__findattr_ex__(name);
	}

	public PyString __str__() {
		//System.out.println("PyCPeer__str__");
		//Object er = JyNI.PyObjectAsPyString(objectHandle);
		//return (PyString) JyNI.repr(objectHandle);
		PyString er = JyNI.PyObjectAsPyString(objectHandle);
		return er == null ? (PyString) JyNI.repr(objectHandle) : er;
	}
	
	public PyString __repr__() {
		//System.out.println("PyCPeer__repr__");
		return (PyString) JyNI.repr(objectHandle);
	}

	public String toString()
	{
		return JyNI.PyObjectAsString(objectHandle);
	}

	/**
	 * Though it is discouraged in JNI-documentation,
	 * we use finalize to tidy up the native references
	 * of this peer. We plan to replace this by
	 * a better solution in the future.
	 * (This will probably be as follows:
	 * Track all peers in a Set using WeakReferences
	 * and have these references registered in a ReferenceQueue.
	 * From Time to time poll things from the queue and tidy
	 * up or have a thread permanently waiting on the queue.)
	 */
	protected void finalize() //throws Throwable
	{
		if (objectHandle != 0) JyNI.clearPyCPeer(objectHandle, 0);
	}
}
