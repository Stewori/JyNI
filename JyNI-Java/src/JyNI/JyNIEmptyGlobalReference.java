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

import java.util.ArrayList;
import java.util.List;
import java.lang.ref.WeakReference;

import org.python.core.Py;
import org.python.core.PyList;
import org.python.core.PyObject;
import org.python.modules._weakref.AbstractReference;
import org.python.modules._weakref.ReferenceBackend;

/**
 * We use this class to represent Java-counterparts of native weak
 * references that were already cleared before they were converted
 * to Java-side. Since a user is free to do what she wants with a
 * weak reference she created - also after it was cleared - this is
 * actually a possible scenario.
 * As long as no better idea comes up, we use this dummy-implementation
 * to represent already cleared references. 
 */
public class JyNIEmptyGlobalReference implements ReferenceBackend {
	public static JyNIEmptyGlobalReference defaultInstance =
			new JyNIEmptyGlobalReference();

	protected ArrayList<WeakReference<AbstractReference>> refs =
			new ArrayList<WeakReference<AbstractReference>>();

	public PyObject get() {return null;}
	
	public void add(AbstractReference ref) {
		refs.add(new WeakReference<>(ref));
	}

	public boolean isCleared() {return true;}

	private final AbstractReference getReferenceAt(int idx) {
		WeakReference<AbstractReference> wref = refs.get(idx);
		return wref.get();
	}

	public synchronized AbstractReference find(Class<?> cls) {
		for (int i = refs.size() - 1; i >= 0; i--) {
			AbstractReference r = getReferenceAt(i);
			if (r == null) {
				refs.remove(i);
			} else if (!r.hasCallback() && r.getClass() == cls) {
				return r;
			}
		}
		return null;
	}

	public int pythonHashCode() {
		throw Py.TypeError("weak object has gone away");
	}

	synchronized public PyList refs() {
        List<AbstractReference> list = new ArrayList<>();
        for (int i = refs.size() - 1; i >= 0; i--) {
            AbstractReference r = getReferenceAt(i);
            if (r == null) {
                refs.remove(i);
            } else {
                list.add(r);
            }
        }
        return new PyList(list);
    }

	public void restore(PyObject formerReferent) {}

	synchronized public int count() {
        for (int i = refs.size() - 1; i >= 0; i--) {
            AbstractReference r = getReferenceAt(i);
            if (r == null) {
                refs.remove(i);
            }
        }
        return refs.size();
    }
}
