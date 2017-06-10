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
#
#
#
# makefile for JyNI
#
# Author: Jonathan Hale, Stefan Richthofer
#

CC = gcc
JC = javac
JAVA = java
OUTPUTDIR = ./build

# Adjust the following line to point to Jython 2.7
JYTHON = ./jython.jar
# for instance, if you extracted it to your home folder:
# JYTHON = /home/your_name/jython.jar

# Adjust the path below to match your system, if not yet appropriate
JAVA_HOME ?= /usr/lib/jvm/default-java

#The symlink "default-java" does not exist on every system. If gnumake tells you that the header
#jni.h is missing, please adjust JAVA_HOME appropriately. Example for Java 7, 64 bit:
#JAVA_HOME = /usr/lib/jvm/java-7-openjdk-amd64

PLATFORM = linux

JYNI = ./JyNI-Java/src
JYNIBIN = ./JyNI-Java/bin

INCLUDES = -I./JyNI-C/include -I./JyNI-C/include/Python_JyNI -I./JyNI-Java/include -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/$(PLATFORM) `python2.7-config --includes`
#-I/usr/include/tcl8.6
CFLAGS = -std=gnu89 -fPIC -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wl,--add-stdcall-alias -c $(INCLUDES)
LDFLAGS = -shared
# -ltk8.6 -ltcl8.6
#-L/usr/X11/lib -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib -ltk8.6 -ltcl8.6 -lX11
# You can optionally remove -source 1.7 -target 1.7. It's purpose is to achieve maximal compatibility by default.
JFLAGS= -cp $(JYTHON):$(JYNI) -d $(JYNIBIN) -source 1.7 -target 1.7

SOURCES = $(wildcard JyNI-C/src/*.c) $(wildcard JyNI-C/src/Python/*.c) $(wildcard JyNI-C/src/Objects/*.c) $(wildcard JyNI-C/src/Modules/*.c)
OBJECTS = $(SOURCES:.c=.o)
JSOURCES = $(wildcard JyNI-Java/src/JyNI/*.java) $(wildcard JyNI-Java/src/JyNI/gc/*.java)

all: $(OUTPUTDIR) libJyNI libJyNI-Loader JyNI
	@echo ''
	@echo 'Build successful.'

$(OUTPUTDIR):
	mkdir $(OUTPUTDIR)

.o:
	$(CC) $(CFLAGS) $< -o $@

$(JYTHON):
	@echo ''
	@echo '------------------------------------------------'
	@echo 'Fatal error: Could not find jython.jar.'
	@echo 'Either put jython.jar into the JyNI base folder,
	@echo 'or adjust the JYTHON-variable at the top of'
	@echo 'makefile to point to your installed jython.jar.'
	@echo 'Be sure to use Jython 2.7.1 or newer.'
	@echo '------------------------------------------------'
	@echo ''
	@false

JAVA_HOME_hint:
ifeq "$(wildcard $(JAVA_HOME) )" ""
	@echo 'Getting JAVA_HOME from java...'
	@echo 'To avoid this extra startup-time, set JAVA_HOME explicitly, e.g.'
	@echo 'export JAVA_HOME='"'"'/usr/lib/jvm/java-8-openjdk-amd64'"'"
endif

# We could inline JAVA_HOME_hint into this recipe, but then it wouldn't be flushed before the user waits during the startup-time.
# But the text is actually intended to be read by the user during this time. So we make it a separate recipe to force flushing.
$(JAVA_HOME): JAVA_HOME_hint $(JYTHON)
# For some reason make doesn't recognize $(JAVA_HOME) as existing, even if it exists (maybe because it's a symlink?, but we
# frequently tested with jython.jar being a symlink too and it worked. Maybe because it's no subpath of workdir?)
# However this additional check ensures it is only executed if needed.
ifeq "$(wildcard $(JAVA_HOME) )" ""
	$(eval JAVA_HOME = $(shell $(JAVA) -jar $(JYTHON) -c "from java.lang import System; print System.getProperty('java.home')[:-4]"))
endif

libJyNI: $(JAVA_HOME) $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(OUTPUTDIR)/libJyNI.so

libJyNI-Loader: $(JAVA_HOME) ./JyNI-Loader/JyNILoader.o
	$(CC) $(LDFLAGS) ./JyNI-Loader/JyNILoader.o -o $(OUTPUTDIR)/libJyNI-Loader.so

$(JYNIBIN):
	mkdir $(JYNIBIN)

$(JYNIBIN)/JyNI: $(JYNIBIN)
	$(JC) $(JFLAGS) $(JSOURCES)

$(JYNIBIN)/Lib: $(JYTHON) $(JYNIBIN)
	cp -r JyNI-Lib/* $(JYNIBIN)
	$(JAVA) -cp $(JYTHON) org.python.util.jython -c "import compileall; compileall.compile_dir('$(JYNIBIN)')"

JyNI: $(JYTHON) $(JYNIBIN)/JyNI $(JYNIBIN)/Lib
	cp -r JyNI-Java/META-INF $(JYNIBIN)
	jar cvf $(OUTPUTDIR)/JyNI.jar -C $(JYNIBIN) .

cleanJ:
	rm -rf $(JYNIBIN)

clean:
	rm -rf $(JYNIBIN)
	rm -f ./JyNI-C/src/*.o
	rm -f ./JyNI-C/src/Python/*.o
	rm -f ./JyNI-C/src/Objects/*.o
	rm -f ./JyNI-C/src/Modules/*.o
	rm -f ./JyNI-Loader/JyNILoader.o

.PHONY: JyNI libJyNI libJyNI-Loader clean cleanJ JAVA_HOME_hint all

