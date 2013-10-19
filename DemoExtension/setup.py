#
# Simple build script for DemoExtension
# 
# Author: Jonathan Hale
#

from distutils.core import setup, Extension

DemoExt = Extension('DemoExtension', 
		sources = ['DemoExtensionmodule.c'],
		include_dirs = [],
		libraries = [],
		library_dirs = []
		)

setup( name = 'DemoExtension',
	version = '1.0.0',
	description = 'Demo Extension for JyNI test purposes.',
	author = 'Stefan Richthofer',
	ext_modules = [DemoExt]
)


