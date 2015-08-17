/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
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

package JyNI.gc;

import JyNI.JyNI;
import org.python.core.PyObject;
import org.python.core.PyInstance;
import org.python.core.finalization.*;

public class JyNIFinalizeTriggerFactory implements FinalizeTriggerFactory {

	static class JyNIFinalizeTrigger extends FinalizeTrigger {
		protected JyNIFinalizeTrigger(PyObject obj) {
			super(obj);
		}
	}

	@Override
	public FinalizeTrigger makeTrigger(PyObject toFinalize) {
		FinalizeTrigger result = new JyNIFinalizeTrigger(toFinalize);
		if (toFinalize instanceof PyInstance) {
			if (JyNI.lookupNativeHandle(toFinalize) != 0)
			/*
			 * In this case we deactivate the trigger, because the native
			 * object is responsible for finalization on its dealloc-method.
			 * If the Java-object toFinalize is still valid and alive when
			 * its native counter-part dies, the native object detects this
			 * and won't perform finalization, but instead reactivates this
			 * finalizeTrigger.
			 */
				result.clear();
		}
		return result;
	}
}
