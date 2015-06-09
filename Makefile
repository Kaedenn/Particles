########################################################################
# Template makefile for collision detection example programs.
# Copyright (c) 2005-2012 Oliver Kreylos
#
# When used with the default settings, the makefile will compile all
# files with extensions of .c and .cpp found in any subdirectory of the
# current directory and link them into a single executable program.
# Additionally, the makefile will generate .cpp source files from all
# fltk user interface definition files with extension .fld and link them
# into the executable program as well.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
# 02110-1301, USA.
########################################################################

#
# Information about the program to be built
#

# Program name
TARGET = CollisionBoxTest

# List of external software packages used by program
PACKAGES = GLUT GL
# For Fltk programs:
#PACKAGES = FLTK XFT GL X11

#
# Extra flags passed to the various programs in the program building
# pipeline
#

# Optimization flags
OPTFLAGS = -g -O0 -ggdb

ifdef FAST
  # For debugging builds:
  OPTFLAGS = -g0 -O3
endif

ifdef PROF
  # For profiling builds:
  OPTFLAGS = -g -O0 -pg
endif

# Compilation flags for C source files
CFLAGS = -I.

# Compilation flags for C++ source files
CPPFLAGS = -I. -Wall -Wextra -pthread -std=c++11

# Linker flags
LDFLAGS = -pthread

#
# Extensions used for various source file formats
#

# Extension for C header files
CHEADEREXT = h

# Extension for C source files
CEXT = c

# Extension for C++ header files
CPPHEADEREXT = h

# Extension for C++ source files
CPPEXT = cpp

# Extension for fltk user interface definition files
FLUIDEXT = fld

########################################################################
# Everything below here is set up to work for the CSIF Linux computers
# and can be considered "magic dust."
########################################################################

#
# Settings to tweak the program building pipeline
#

# C compiler to use
CCOMP = gcc

# C++ compiler to use
CPPCOMP = g++

# Linker to use (g++ is safe to link both C and C++ programs)
LD = g++

# Directories where dependency and object files go
DEPDIR = ./d
ifdef FAST
  OBJDIR = ./o
else
  OBJDIR = ./o/debug
endif

# Directory where executable programs go
EXEDIR = .

#
# Definitions of external software packages
#

# Default directory for include files
INCLUDEEXT = include

# Default directory for library files
ifeq ($(shell uname -m),x86_64)
  LIBEXT = lib64
else
  LIBEXT = lib
endif

# Default directory for binary files
BINEXT = bin

# The X11 library
X11_BASE       = /usr/X11R6
X11_INCLUDEDIR = $(X11_BASE)/$(INCLUDEEXT)
X11_LIBDIR     = $(X11_BASE)/$(LIBEXT)
X11_LIBS       = Xmu Xi X11

# The OpenGL library
GL_BASE       = /usr
GL_INCLUDEDIR = $(GL_BASE)/$(INCLUDEEXT)
GL_LIBDIR     = $(GL_BASE)/$(LIBEXT)
GL_LIBS       = GL

# The glut library
GLUT_BASE       = /usr
GLUT_INCLUDEDIR = $(GLUT_BASE)/$(INCLUDEEXT)
GLUT_LIBDIR     = $(GLUT_BASE)/$(LIBEXT)
GLUT_LIBS       = glut GLU

# The X antialiased font library
XFT_BASE       = /usr/X11R6
XFT_INCLUDEDIR = $(XFT_BASE)/$(INCLUDEEXT)
XFT_LIBDIR     = $(XFT_BASE)/$(LIBEXT)
XFT_LIBS       = Xft

# The fltk library
FLTK_BASE       = /usr
FLTK_INCLUDEDIR = $(FLTK_BASE)/$(INCLUDEEXT)
FLTK_LIBDIR     = $(FLTK_BASE)/$(LIBEXT)
FLTK_LIBS       = fltk fltk_images fltk_forms fltk_gl
FLTK_BINDIR     = $(FLTK_BASE)/$(BINEXT)

# Create list of include directories:
INCLUDEDIRS = $(patsubst %,-I%,$(foreach PACKAGE,$(PACKAGES),$($(PACKAGE)_INCLUDEDIR)))

# Create list of library search directories:
LIBDIRS = $(patsubst %,-L%,$(foreach PACKAGE,$(PACKAGES),$($(PACKAGE)_LIBDIR)))

# Create list of linked-in libraries:
LIBS = $(patsubst %,-l%,$(foreach PACKAGE,$(PACKAGES),$($(PACKAGE)_LIBS)))

#
# Rules to preprocess various files into C/C++ files
#

# make rule to "compile" fluid user interface generator files into C++ source files:
%.$(CPPHEADEREXT) %.$(CPPEXT): %.$(FLUIDEXT)
	$(FLTK_BINDIR)/fluid -c -h $*.$(CPPHEADEREXT) -o $*.$(CPPEXT) $<

#
# Rules and macros to compile C/C++ source files with on-the-fly dependency generation
#

# sed command to process an automatically generated dependency file:
DEPFILETEMPLATE = '$(patsubst %.o,%.d,$@)'
define PROCESS_DEPFILE
sed -e 's!\([^:]*:\)!$$(OBJDIR)/$*.o:!g' -e 's!/usr/[^ ]*!!g' -e '/^[ ]*\\$$/ d' < $(DEPFILETEMPLATE) > $(DEPDIR)/$*.d
endef

# make rule to compile C source code:
CCFLAGS = $(OPTFLAGS) $(INCLUDEDIRS) $(CFLAGS)
$(OBJDIR)/%.o: %.$(CEXT)
	@mkdir -p $(DEPDIR)/$(*D)
	@mkdir -p $(OBJDIR)/$(*D)
	$(CCOMP) -MD -c -o $@ $(CCFLAGS) $<
	@$(PROCESS_DEPFILE)
	@rm -f $(DEPFILETEMPLATE)

# make rule to compile C++ source code:
CCPPFLAGS = $(OPTFLAGS) $(INCLUDEDIRS) $(CPPFLAGS)
$(OBJDIR)/%.o: %.$(CPPEXT)
	@mkdir -p $(DEPDIR)/$(*D)
	@mkdir -p $(OBJDIR)/$(*D)
	$(CPPCOMP) -MD -c -o $@ $(CCPPFLAGS) $<
	@$(PROCESS_DEPFILE)
	@rm -f $(DEPFILETEMPLATE)

# make rule to build and run target by default:
ifndef FAST
  TARGETNAME = $(TARGET).debug
else
  TARGETNAME = $(TARGET)
endif
target: $(TARGETNAME)

# make rule to remove artifacts of compilation:
.PHONY: clean
clean:
	-rm -rf $(DEPDIR)
	-rm -rf $(OBJDIR)
	-rm -f $(FLUID_SOURCES:%.$(FLUIDEXT)=%.$(CPPHEADEREXT)) $(FLUID_SOURCES:%.$(FLUIDEXT)=%.$(CPPEXT))
	-rm -f $(TARGET) $(TARGET).debug
	-rm -f core.* core

# Use empty default target to treat missing prerequisites as outdated
.DEFAULT: ;

# include all automatically generated dependency files:
DEPFILES = $(shell find $(DEPDIR) -follow -name "*.d" 2>/dev/null)
include $(DEPFILES)

#
# Build program by compiling and linking all C/C++ source files
# underneath the current directory
#

# Get list of all C source files underneath the current directory:
C_SOURCES = $(shell find . -follow -name "*.$(CEXT)")
C_OBJECTS = $(C_SOURCES:%.$(CEXT)=$(OBJDIR)/%.o)

# Get list of all fluid UI definition files underneath the current directory:
FLUID_SOURCES = $(shell find . -follow -name "*.$(FLUIDEXT)")
FLUID_OBJECTS = $(FLUID_SOURCES:%.$(FLUIDEXT)=%.$(CPPEXT))

# Mark C++ source files created by fluid as intermediate (delete them when done compiling):
.INTERMEDIATE: $(FLUID_SOURCES:%.$(FLUIDEXT)=%.$(CPPHEADEREXT)) $(FLUID_SOURCES:%.$(FLUIDEXT)=%.$(CPPEXT))

# Get list of all C++ source files underneath the current directory:
CPP_SOURCES = $(FLUID_OBJECTS) $(shell find . -follow -name "*.$(CPPEXT)")
CPP_OBJECTS = $(CPP_SOURCES:%.$(CPPEXT)=$(OBJDIR)/%.o)

# make rule to build program from all object files:
$(EXEDIR)/$(TARGETNAME): $(C_OBJECTS) $(CPP_OBJECTS)
	@mkdir -p $(EXEDIR)/$(*D)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBDIRS) $(LIBS)

