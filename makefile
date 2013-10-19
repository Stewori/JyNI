#
# makefile for JyNI
#
# Author: Jonathan Hale, Stefan Richthofer
#

CC = gcc
JC = javac
OUTPUTDIR = ./build

# Adjust the following line to point to Jython 2.7
JYTHON = ./jython.jar
#for instance, if you extracted it to your home folder:
#JYTHON = /home/your_name/jython.jar

# Adjust the two paths below to match your system, if not yet appropriate
PY_INCLUDE = /usr/include/python2.7
JAVA_HOME = /usr/lib/jvm/default-java

JYNI = ./JyNI-Java/src
JYNIBIN = ./JyNI-Java/bin
INCLUDES = -I./JyNI-C/include -I./JyNI-C/include/Python_JyNI -I./JyNI-Java/include -I$(JAVA_HOME)/include -I$(PY_INCLUDE)
CFLAGS = -fPIC -Wl,--add-stdcall-alias -c $(INCLUDES)
LDFLAGS = -shared
JFLAGS= -cp $(JYTHON):$(JYNI) -d $(JYNIBIN)

SOURCES = $(wildcard JyNI-C/src/*.c)
OBJECTS = $(SOURCES:.c=.o)
JSOURCES = $(wildcard JyNI-Java/src/JyNI/*.java)

all: $(OUTPUTDIR) libJyNI libJyNI-Loader JyNI
	@echo ''
	@echo 'Build finnished.'

$(OUTPUTDIR):
	mkdir $(OUTPUTDIR)

.o:
	$(CC) $(CFLAGS) $< -o $@

libJyNI: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(OUTPUTDIR)/libJyNI.so

libJyNI-Loader: ./JyNI-Loader/JyNILoader.o
	$(CC) $(LDFLAGS) ./JyNI-Loader/JyNILoader.o -o $(OUTPUTDIR)/libJyNI-Loader.so

$(JYNIBIN)/JyNI:
	$(JC) $(JFLAGS) $(JSOURCES)

$(JYTHON):
	@echo ''
	@echo '----------------------------------------------'
	@echo 'Fatal error: Could not find jython.jar.'
	@echo 'Either put jython.jar into the JyNI base folder, or adjust the JYTHON-variable at the top of makefile to point to your installed jython.jar.'
	@echo 'Be sure to use Jython 2.7 (beta) or newer.'
	@echo '----------------------------------------------'
	@echo ''
	@false

JyNI: $(JYTHON) $(JYNIBIN)/JyNI
	jar cvf $(OUTPUTDIR)/JyNI.jar -C $(JYNIBIN) .

clean:
	rm -rf $(JYNIBIN)/JyNI
	rm -f ./JyNI-C/src/*.o
	rm -f ./JyNI-Loader/JyNILoader.o

.PHONY: JyNI libJyNI libJyNI-Loader clean all

