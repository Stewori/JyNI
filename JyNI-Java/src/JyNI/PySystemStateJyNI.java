/*
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014, 2015 Python Software Foundation.  All rights reserved.
 * 
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015 Stefan Richthofer.  All rights reserved.
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

import org.python.core.PySystemState;
import org.python.util.PythonInterpreter;

/**
 * A variant of PySystemState that supports the sys-functions
 * 
 * sys.setdlopenflags and sys.getdlopenflags.
 * 
 * These are relevant for loading CPython Extensions via JyNI.
 * The JyNI initializer adds these functions to the default
 * PySystemState anyway.
 * The sole purpose of this class is to allow for
 * PythonInterpreter-setups using a custom PySystemState, f.i.
 * 
 * PythonInterpreter pint = new PythonInterpreter(new PySystemStateJyNI());
 * 
 * This is equivalent to calling
 * 
 * PythonInterpreter pint = new PythonInterpreter(new PySystemState());
 * pint.exec("import sys");
 * pint.exec("import JyNI.JyNI");
 * pint.exec("sys.dlopenflags = JyNI.JyNI.RTLD_NOW");
 * pint.exec("def setdlopenflags(n): sys.dlopenflags = n");
 * pint.exec("sys.setdlopenflags = setdlopenflags");
 * pint.exec("sys.getdlopenflags = lambda: sys.dlopenflags");
 * 
 * @author Stefan Richthofer
 *
 */
public class PySystemStateJyNI extends PySystemState {
	//protected int dlopenflags = JyNI.RTLD_NOW;
	//public int getdlopenflags() {return dlopenflags;}
	//public void setdlopenflags(int n) {this.dlopenflags = n;}
	
	public PySystemStateJyNI()
	{
		super();
		PythonInterpreter pint = new PythonInterpreter(this);
		pint.exec("import sys");
		pint.exec("import JyNI.JyNI");
		pint.exec("sys.dlopenflags = JyNI.JyNI.RTLD_JyNI_DEFAULT");
		//pint.exec("sys.setdlopenflags = JyNI.JyNI.setDLOpenFlags");
		//pint.exec("sys.getdlopenflags = JyNI.JyNI.getDLOpenFlags");
		//pint.exec("sys.setdlopenflags = lambda n: (sys.dlopenflags = n)");
		pint.exec("def setdlopenflags(n): sys.dlopenflags = n");
		pint.exec("sys.setdlopenflags = setdlopenflags");
		pint.exec("sys.getdlopenflags = lambda: sys.dlopenflags");
		pint.cleanup();
	}
}
