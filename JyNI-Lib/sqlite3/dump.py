#
#  This File is based on sqlite3/dump.py from CPython 2.7.9.
#  It has been modified to suit JyNI needs.
#
#  Copyright of JyNI:
#  Copyright (c) 2013, 2014, 2015, 2016, 2017 Stefan Richthofer.
#  All rights reserved.
#
#
#  Copyright of Python and Jython:
#  Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
#  2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
#  Python Software Foundation.
#  All rights reserved.
#
#
#  This file is part of JyNI.
#
#  JyNI is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as
#  published by the Free Software Foundation, either version 3 of
#  the License, or (at your option) any later version.
#
#  JyNI is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with JyNI.  If not, see <http://www.gnu.org/licenses/>.


# Mimic the sqlite3 console shell's .dump command
# Author: Paul Kippes <kippesp@gmail.com>

# Every identifier in sql is quoted based on a comment in sqlite
# documentation "SQLite adds new keywords from time to time when it
# takes on new features. So to prevent your code from being broken by
# future enhancements, you should normally quote any identifier that
# is an English language word, even if you do not have to."

def _iterdump(connection):
    """
    Returns an iterator to the dump of the database in an SQL text format.

    Used to produce an SQL dump of the database.  Useful to save an in-memory
    database for later restoration.  This function should not be called
    directly but instead called from the Connection method, iterdump().
    """

    cu = connection.cursor()
    yield('BEGIN TRANSACTION;')

    # sqlite_master table contains the SQL CREATE statements for the database.
    q = """
        SELECT "name", "type", "sql"
        FROM "sqlite_master"
            WHERE "sql" NOT NULL AND
            "type" == 'table'
            ORDER BY "name"
        """
    schema_res = cu.execute(q)
    for table_name, type, sql in schema_res.fetchall():
        if table_name == 'sqlite_sequence':
            yield('DELETE FROM "sqlite_sequence";')
        elif table_name == 'sqlite_stat1':
            yield('ANALYZE "sqlite_master";')
        elif table_name.startswith('sqlite_'):
            continue
        # NOTE: Virtual table support not implemented
        #elif sql.startswith('CREATE VIRTUAL TABLE'):
        #    qtable = table_name.replace("'", "''")
        #    yield("INSERT INTO sqlite_master(type,name,tbl_name,rootpage,sql)"\
        #        "VALUES('table','{0}','{0}',0,'{1}');".format(
        #        qtable,
        #        sql.replace("''")))
        else:
            yield('%s;' % sql)

        # Build the insert statement for each row of the current table
        table_name_ident = table_name.replace('"', '""')
        res = cu.execute('PRAGMA table_info("{0}")'.format(table_name_ident))
        column_names = [str(table_info[1]) for table_info in res.fetchall()]
        q = """SELECT 'INSERT INTO "{0}" VALUES({1})' FROM "{0}";""".format(
            table_name_ident,
            ",".join("""'||quote("{0}")||'""".format(col.replace('"', '""')) for col in column_names))
        query_res = cu.execute(q)
        for row in query_res:
            yield("%s;" % row[0])

    # Now when the type is 'index', 'trigger', or 'view'
    q = """
        SELECT "name", "type", "sql"
        FROM "sqlite_master"
            WHERE "sql" NOT NULL AND
            "type" IN ('index', 'trigger', 'view')
        """
    schema_res = cu.execute(q)
    for name, type, sql in schema_res.fetchall():
        yield('%s;' % sql)

    yield('COMMIT;')
