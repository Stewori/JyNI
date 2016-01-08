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
import org.python.core.JyAttribute;
import org.python.modules._weakref.GlobalRef;
import org.python.modules._weakref.ReferenceBackend;
import org.python.modules._weakref.ReferenceBackendFactory;

public class JyNIGlobalRefFactory implements ReferenceBackendFactory {
	public ReferenceBackend makeBackend(GlobalRef caller, PyObject referent) {
		if (referent == null ||
				JyAttribute.hasAttr(referent, JyAttribute.WEAK_REF_ATTR))
			return new JyNIGlobalRef(caller);
		else
			return caller;
	}

	//Note: Currently only called by GlobalRef.call()
	public void notifyClear(ReferenceBackend ref, GlobalRef caller) {
		((JyNIGlobalRef) ref).notifyBackendClear(caller);
	}

	public void updateBackend(ReferenceBackend ref, GlobalRef caller) {
		((JyNIGlobalRef) ref).updateBackend(caller);
	}
}
