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

/**
 * This class provides a basic lock implementation equivalent to
 * org.python.modules.thread.PyLock, but without the overhead of
 * inheriting PyObject. This basic lock is intended to provide the
 * locking features of PyThread.h.
 *
 * @author Stefan Richthofer
 *
 */
public class JyLock {

    private boolean locked = false;

    public boolean acquire() {
        return acquire(true);
    }

    public synchronized boolean acquire(boolean waitflag) {
        if (waitflag) {
            while (locked) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    //System.err.println("Interrupted thread");
                }
            }
            locked = true;
            return true;
        } else {
            if (locked) {
                return false;
            } else {
                locked = true;
                return true;
            }
        }
    }

    public synchronized void release() {
        if (locked) {
            locked = false;
            notifyAll();
        } else {
            throw new IllegalStateException("lock not acquired");
        }
    }

    public boolean locked() {
        return locked;
    }
}