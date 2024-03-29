##############################################################################
# Product: Makefile for QUTEST-QP/C++ for Windows and POSIX *HOSTS*
# Last updated for version 6.5.1
# Last updated on  2019-05-24
#
#                    Q u a n t u m  L e a P s
#                    ------------------------
#                    Modern Embedded Software
#
# Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Alternatively, this program may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GNU General Public License and are specifically designed for
# licensees interested in retaining the proprietary status of their code.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Contact information:
# https://www.state-machine.com
# mailto:info@state-machine.com
##############################################################################
#
# examples of invoking this Makefile:
# make         # make and run the Python tests in the current directory
# make TESTS=test*.py  # make and run the selected tests in the curr. dir.
# make SCRIPT=tcl # make and run the Tcl tests
# make HOST=localhost:7705 # connect to host:port
# make norun   # only make but not run the tests
# make clean   # cleanup the build
# make debug   # only run tests in DEBUG mode
#
# NOTE:
# To use this Makefile on Windows, you will need the GNU make utility, which
# is included in the Qtools collection for Windows, see:
#    http://sourceforge.net/projects/qpc/files/QTools/
#

#-----------------------------------------------------------------------------
# project name:
#
PROJECT := test_qutest

#-----------------------------------------------------------------------------
# project directories:
#

# list of all source directories used by this project
VPATH := . \
	../src

# list of all include directories needed by this project
INCLUDES := -I. \
	-I../src

# location of the QP/C++ framework (if not provided in an env. variable)
ifeq ($(QPCPP),)
QPCPP := ../../../..
endif

# make sure that QTOOLS env. variable is defined...
ifeq ("$(wildcard $(QTOOLS))","")
$(error QTOOLS not found. Please install QTools and define QTOOLS env. variable)
endif

#-----------------------------------------------------------------------------
# project files:
#

# C source files...
C_SRCS :=

# C++ source files...
CPP_SRCS := \
	test_qutest.cpp

LIB_DIRS :=
LIBS     :=

# defines...
DEFINES  :=

#-----------------------------------------------------------------------------
# add QP/C++ framework (depends on the OS this Makefile runs on):
#
ifeq ($(OS),Windows_NT)
	QP_PORT_DIR := $(QPCPP)/ports/win32-qutest
	LIB_DIRS += -L$(QP_PORT_DIR)/mingw
	LIBS     += -lqp -lws2_32
else
	QP_PORT_DIR := $(QPCPP)/ports/posix-qutest
	CPP_SRCS += \
	qep_hsm.cpp \
	qep_msm.cpp \
	qf_act.cpp \
	qf_actq.cpp \
	qf_defer.cpp \
	qf_dyn.cpp \
	qf_mem.cpp \
	qf_ps.cpp \
	qf_qact.cpp \
	qf_qeq.cpp \
	qf_qmact.cpp \
	qf_time.cpp \
	qs.cpp \
	qs_64bit.cpp \
	qs_rx.cpp \
	qs_fp.cpp \
	qutest.cpp \
	qutest_port.cpp

	LIBS += -lpthread
endif

#============================================================================
# Typically you should not need to change anything below this line

VPATH    += $(QPCPP)/src/qf $(QPCPP)/src/qs $(QP_PORT_DIR)
INCLUDES += -I$(QPCPP)/include -I$(QPCPP)/src -I$(QP_PORT_DIR)

#-----------------------------------------------------------------------------
# GNU toolset:
#
# NOTE:
# GNU toolset (MinGW) is included in the QTools collection for Windows, see:
#     http://sourceforge.net/projects/qpc/files/QTools/
# It is assumed that %QTOOLS%\bin directory is added to the PATH
#
CC    := gcc
CPP   := g++
#LINK  := gcc    # for C programs
LINK  := g++   # for C++ programs

ifeq ($(SCRIPT),tcl)
QUTEST := tclsh $(QTOOLS)/qspy/tcl/qutest.tcl
else # default
QUTEST := python $(QTOOLS)/qspy/py/qutest.py
endif

#-----------------------------------------------------------------------------
# basic utilities (depends on the OS this Makefile runs on):
#
ifeq ($(OS),Windows_NT)
	MKDIR      := mkdir
	RM         := rm
	TARGET_EXT := .exe
else ifeq ($(OSTYPE),cygwin)
	MKDIR      := mkdir -p
	RM         := rm -f
	TARGET_EXT := .exe
else
	MKDIR      := mkdir -p
	RM         := rm -f
	TARGET_EXT :=
endif

#-----------------------------------------------------------------------------
# build options...

BIN_DIR := build

CFLAGS  := -c -g -O -fno-pie -std=c99 -pedantic -Wall -Wextra -W \
	$(INCLUDES) $(DEFINES) -DQ_SPY -DQ_UTEST -DQ_HOST

CPPFLAGS := -c -g -O -fno-pie -std=c++11 -pedantic -Wall -Wextra \
	-fno-rtti -fno-exceptions \
	$(INCLUDES) $(DEFINES) -DQ_SPY -DQ_UTEST -DQ_HOST

LINKFLAGS := -no-pie

#-----------------------------------------------------------------------------
C_OBJS       := $(patsubst %.c,%.o,   $(C_SRCS))
CPP_OBJS     := $(patsubst %.cpp,%.o, $(CPP_SRCS))

TARGET_EXE   := $(BIN_DIR)/$(PROJECT)$(TARGET_EXT)
C_OBJS_EXT   := $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT   := $(patsubst %.o,%.d, $(C_OBJS_EXT))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o,%.d, $(CPP_OBJS_EXT))


#-----------------------------------------------------------------------------
# rules
#

.PHONY : norun debug clean show

ifeq ($(MAKECMDGOALS),norun)
all : $(TARGET_EXE)
norun : all
else
all : $(TARGET_EXE) run
endif

ifeq (, $(TESTS))
ifeq ($(SCRIPT),tcl)
TESTS := *.tcl
else
TESTS := *.py
endif
endif

$(TARGET_EXE) : $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(CPP) $(CPPFLAGS) $(QPCPP)/include/qstamp.cpp -o $(BIN_DIR)/qstamp.o
	$(LINK) $(LINKFLAGS) $(LIB_DIRS) -o $@ $^ $(BIN_DIR)/qstamp.o $(LIBS)

run : $(TARGET_EXE)
	$(QUTEST) $(TESTS) $(TARGET_EXE) $(HOST)

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $< > $@

$(BIN_DIR)/%.d : %.c
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $< > $@

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

# create BIN_DIR and include dependencies only if needed
ifneq ($(MAKECMDGOALS),clean)
  ifneq ($(MAKECMDGOALS),show)
     ifneq ($(MAKECMDGOALS),debug)
ifeq ("$(wildcard $(BIN_DIR))","")
$(shell $(MKDIR) $(BIN_DIR))
endif
-include $(C_DEPS_EXT) $(CPP_DEPS_EXT)
     endif
  endif
endif

debug :
	$(QUTEST) $(TESTS) DEBUG $(HOST)

clean :
	-$(RM) $(BIN_DIR)/*.o \
	$(BIN_DIR)/*.d \
	$(TARGET_EXE)

show :
	@echo PROJECT      = $(PROJECT)
	@echo TARGET_EXE   = $(TARGET_EXE)
	@echo VPATH        = $(VPATH)
	@echo C_SRCS       = $(C_SRCS)
	@echo CPP_SRCS     = $(CPP_SRCS)
	@echo C_DEPS_EXT   = $(C_DEPS_EXT)
	@echo C_OBJS_EXT   = $(C_OBJS_EXT)
	@echo C_DEPS_EXT   = $(C_DEPS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
	@echo CPP_OBJS_EXT = $(CPP_OBJS_EXT)
	@echo LIB_DIRS     = $(LIB_DIRS)
	@echo LIBS         = $(LIBS)
	@echo DEFINES      = $(DEFINES)
	@echo QTOOLS       = $(QTOOLS)
	@echo QUTEST       = $(QUTEST)
	@echo HOST         = $(HOST)
	@echo TESTS        = $(TESTS)

