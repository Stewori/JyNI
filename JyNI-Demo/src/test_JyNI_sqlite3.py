'''
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


Created on 29.07.2016

@author: Stefan Richthofer
'''

import sys
import os
import unittest
# class TestJyNI(unittest.TestCase):
# for line in sys.path:
# 	print line
import sqlite3

class Test_sqlite3(unittest.TestCase):

	def test_connection_basic(self):
		try:
			os.remove('test.db')
		except:
			pass
		conn = sqlite3.connect('test.db')
# 		print "Opened database successfully";

		conn.execute('''CREATE TABLE GALAXY
				(ID INT PRIMARY KEY      NOT NULL,
				NAME             TEXT    NOT NULL,
				DIAMETER_LY      INT     NOT NULL,
				TYPE             CHAR(50),
				DIST_EARTH_MLY   REAL);''')
# 		print "Table created successfully";


		conn.execute("INSERT INTO GALAXY (ID,NAME,DIAMETER_LY,TYPE,DIST_EARTH_MLY) \
				VALUES (1, 'Milky Way', 100000, 'SBbc', 0.027 )");

		conn.execute("INSERT INTO GALAXY (ID,NAME,DIAMETER_LY,TYPE,DIST_EARTH_MLY) \
				VALUES (2, 'Andromeda Galaxy', 220000, 'SA(s)b', 2.56 )");

		conn.execute("INSERT INTO GALAXY (ID,NAME,DIAMETER_LY,TYPE,DIST_EARTH_MLY) \
				VALUES (3, 'Triangulum Galaxy', 60000, 'SAc', 2.64 )");

		conn.execute("INSERT INTO GALAXY (ID,NAME,DIAMETER_LY,TYPE,DIST_EARTH_MLY) \
				VALUES (4, 'Barnards Galaxy', 7000, 'IB(s)m IV-V', 1.630 )");

		conn.commit()
# 		print "Records created successfully";


		cursor = conn.execute("SELECT id, name, diameter_ly, type  from GALAXY")
		rowstr = []
		for row in cursor:
			rowstr.append(str(row))

# 		for line in rowstr:
# 			print line

		self.assertEqual(rowstr[0], "(1, u'Milky Way', 100000, u'SBbc')")
		self.assertEqual(rowstr[1], "(2, u'Andromeda Galaxy', 220000, u'SA(s)b')")
		self.assertEqual(rowstr[2], "(3, u'Triangulum Galaxy', 60000, u'SAc')")
		self.assertEqual(rowstr[3], "(4, u'Barnards Galaxy', 7000, u'IB(s)m IV-V')")

		conn.execute("UPDATE GALAXY set DIAMETER_LY = 180000 where ID=1")
		conn.commit
# 		print "Total number of rows updated :", conn.total_changes

		cursor = conn.execute("SELECT id, name, diameter_ly, type  from GALAXY")

		rowstr = []
		for row in cursor:
			rowstr.append(str(row))

		self.assertEqual(rowstr[0], "(1, u'Milky Way', 180000, u'SBbc')")
		self.assertEqual(rowstr[1], "(2, u'Andromeda Galaxy', 220000, u'SA(s)b')")
		self.assertEqual(rowstr[2], "(3, u'Triangulum Galaxy', 60000, u'SAc')")
		self.assertEqual(rowstr[3], "(4, u'Barnards Galaxy', 7000, u'IB(s)m IV-V')")

		conn.execute("DELETE from GALAXY where ID=2;")
		conn.commit
# 		print "Total number of rows deleted :", conn.total_changes

		cursor = conn.execute("SELECT id, name, diameter_ly, type  from GALAXY")
		rowstr = []
		for row in cursor:
			rowstr.append(str(row))

		self.assertEqual(rowstr[0], "(1, u'Milky Way', 180000, u'SBbc')")
		self.assertEqual(rowstr[1], "(3, u'Triangulum Galaxy', 60000, u'SAc')")
		self.assertEqual(rowstr[2], "(4, u'Barnards Galaxy', 7000, u'IB(s)m IV-V')")
		
# 		for line in rowstr:
# 			print line

		conn.close()
		os.remove('test.db')


if __name__ == '__main__':
	unittest.main()
