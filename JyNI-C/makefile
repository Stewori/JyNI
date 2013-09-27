#
# Simple makefile for libJyNI
#
# Author: Jonathan Hale
#

CC = gcc
OUTPUTDIR = ./build

# Adjust the two paths below to match your system
PY_INCLUDE=/usr/include/python2.7/
JAVA_HOME=/usr/lib/jvm/java-7-openjdk-i386

INCLUDES = -I./include/ -I./include/Python_JyNI/ -I../JyNI-Java/include/ -I$(JAVA_HOME)/include/ -I$(PY_INCLUDE)

CFLAGS = -Wl,--add-stdcall-alias -c $(INCLUDES)
LDFLAGS = -shared -fPIC


SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)



all: libJyNI libJyNI-loader

.o:
	$(CC) $(CFLAGS) $< -o $@

libJyNI: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(OUTPUTDIR)/libJyNI.so

libJyNI-loader: ../JyNI-Loader/JyNILoader.o
	$(CC) $(LDFLAGS) ../JyNI-Loader/JyNILoader.o -o $(OUTPUTDIR)/libJyNI-loader.so

clean:
	rm -rf *.o

.PHONY: libJyNI libJyNI-loader clean all
