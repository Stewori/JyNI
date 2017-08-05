/* This File is based on errmap.h from CPython 2.7.13 release.
 * It has been modified to suit JyNI needs.
 *
 *
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

/*
 * JyNI note:
 * The note "Generated file. Do not edit." below is copied from CPython.
 * Actually we did not generate this file, but manually keep track with
 * the CPython version for now.
 * (Todo: Adopt this bit of the build process from CPython properly.)
 */

/* Generated file. Do not edit. */
int winerror_to_errno(int winerror)
{
    switch(winerror) {
        case 2: return 2;
        case 3: return 2;
        case 4: return 24;
        case 5: return 13;
        case 6: return 9;
        case 7: return 12;
        case 8: return 12;
        case 9: return 12;
        case 10: return 7;
        case 11: return 8;
        case 15: return 2;
        case 16: return 13;
        case 17: return 18;
        case 18: return 2;
        case 19: return 13;
        case 20: return 13;
        case 21: return 13;
        case 22: return 13;
        case 23: return 13;
        case 24: return 13;
        case 25: return 13;
        case 26: return 13;
        case 27: return 13;
        case 28: return 13;
        case 29: return 13;
        case 30: return 13;
        case 31: return 13;
        case 32: return 13;
        case 33: return 13;
        case 34: return 13;
        case 35: return 13;
        case 36: return 13;
        case 53: return 2;
        case 65: return 13;
        case 67: return 2;
        case 80: return 17;
        case 82: return 13;
        case 83: return 13;
        case 89: return 11;
        case 108: return 13;
        case 109: return 32;
        case 112: return 28;
        case 114: return 9;
        case 128: return 10;
        case 129: return 10;
        case 130: return 9;
        case 132: return 13;
        case 145: return 41;
        case 158: return 13;
        case 161: return 2;
        case 164: return 11;
        case 167: return 13;
        case 183: return 17;
        case 188: return 8;
        case 189: return 8;
        case 190: return 8;
        case 191: return 8;
        case 192: return 8;
        case 193: return 8;
        case 194: return 8;
        case 195: return 8;
        case 196: return 8;
        case 197: return 8;
        case 198: return 8;
        case 199: return 8;
        case 200: return 8;
        case 201: return 8;
        case 202: return 8;
        case 206: return 2;
        case 215: return 11;
        case 1816: return 12;
        default: return EINVAL;
    }
}
