##############################################################################
# Product: Makefile for QP/C++, DPP on Arduino-UNO, QV kernel, GNU-ARM
# Last Updated for Version: 5.5.0
# Date of the Last Update:  2015-09-24
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
# http://www.state-machine.com
# mailto:info@state-machine.com
##############################################################################
# examples of invoking this Makefile:
# building configurations: Debug (default), Release, and Spy
# make
# make CONF=rel
# make CONF=spy
#
# cleaning configurations: Debug (default), Release, and Spy
# make clean
# make CONF=rel clean
# make CONF=spy clean
#
# NOTE:
# To use this Makefile on Windows, you will need the GNU make utility, which
# is included in the Qtools collection for Windows, see:
#    http://sourceforge.net/projects/qpc/files/Qtools/
#

#-----------------------------------------------------------------------------
# project name
#
PROJECT     := dpp-qv

#-----------------------------------------------------------------------------
# project directories
#

# location of the QP/C++ framework (if not provided in an environemnt var.)
ifeq ($(QPCPP),)
QPCPP := ../../../../..
endif

# QP port used in this project
QP_PORT_DIR := $(QPCPP)/ports/avr/qv/gnu

# list of all source directories used by this project
VPATH = \
	.. \
	../.. \
	$(QPCPP)/source \
	$(QP_PORT_DIR)

# list of all include directories needed by this project
INCLUDES  = \
	-I../.. \
	-I$(QPCPP)/include \
	-I$(QPCPP)/source \
	-I$(QP_PORT_DIR)

#-----------------------------------------------------------------------------
# files
#

# assembler source files
ASM_SRCS :=

# C source files
C_SRCS :=

# C++ source files
CPP_SRCS :=	 \
	bsp.cpp \
	main.cpp \
	philo.cpp \
	table.cpp

OUTPUT    := $(PROJECT)
LD_SCRIPT :=

QP_SRCS := \
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
	qv.cpp

QS_SRCS := \
	qs.cpp \
	qs_rx.cpp \
	qs_fp.cpp

LIB_DIRS  :=
LIBS      :=

# defines
DEFINES   :=

# AVR MCU type
AVR_MCU=atmega328p

#-----------------------------------------------------------------------------
# GNU-AVR toolset (NOTE: You need to adjust to your machine)
# see http://sourceforge.net/projects/winavr
#
ifeq ($(GNU_AVR),)
GNU_AVR = C:\tools\WinAVR
endif

# make sure that the GNU-AVR toolset exists...
ifeq ("$(wildcard $(GNU_AVR))","")
$(error GNU_AVR toolset not found. Please adjust the Makefile)
endif

CC    := $(GNU_AVR)/bin/avr-gcc
CPP   := $(GNU_AVR)/bin/avr-g++
AS    := $(GNU_AVR)/bin/avr-as
LINK  := $(GNU_AVR)/bin/avr-g++
BIN   := $(GNU_AVR)/bin/avr-objcopy

##############################################################################
# Typically, you should not need to change anything below this line

# basic utilities (included in Qtools for Windows), see:
#    http://sourceforge.net/projects/qpc/files/Qtools

MKDIR := mkdir
RM    := rm

#-----------------------------------------------------------------------------
# build options for various configurations for ARM Cortex-M4F
#

# combine all the necessary soruces
VPATH    += $(QPCPP)/3rd_party/gnu_cpp
CPP_SRCS += $(QP_SRCS) $(QP_PORT) mini_cpp.cpp
ASM_SRCS += $(QP_ASMS)



ifeq (rel, $(CONF)) # Release configuration ..................................

BIN_DIR := rel

ASFLAGS =

CFLAGS = -mmcu=$(AVR_MCU) -std=gnu99 -fshort-enums -Wall \
	-ffunction-sections -fdata-sections \
    -Os $(INCLUDES) $(DEFINES) -DNDEBUG

CPPFLAGS = -mmcu=$(AVR_MCU) -fshort-enums -Wall \
	-ffunction-sections -fdata-sections -fno-rtti -fno-exceptions \
    -Os $(INCLUDES) $(DEFINES) -DNDEBUG

else ifeq (spy, $(CONF))  # Spy configuration ................................

BIN_DIR := spy

CPP_SRCS += $(QS_SRCS)

ASFLAGS =

CFLAGS = -mmcu=$(AVR_MCU) -std=gnu99 -fshort-enums -Wall \
	-ffunction-sections -fdata-sections \
    -O $(INCLUDES) $(DEFINES) -DQ_SPY

CPPFLAGS = -mmcu=$(AVR_MCU) -fshort-enums -Wall \
	-ffunction-sections -fdata-sections -fno-rtti -fno-exceptions \
    -O $(INCLUDES) $(DEFINES) -DQ_SPY

else                     # default Debug configuration .......................

BIN_DIR := dbg

ASFLAGS =

CFLAGS = -mmcu=$(AVR_MCU) -std=gnu99 -fshort-enums -Wall \
	-ffunction-sections -fdata-sections \
    -O $(INCLUDES) $(DEFINES)

CPPFLAGS = -mmcu=$(AVR_MCU) -fshort-enums -Wall \
	-ffunction-sections -fdata-sections -fno-rtti -fno-exceptions \
    -O $(INCLUDES) $(DEFINES)

endif

LINKFLAGS = -mmcu=$(AVR_MCU) \
	-Wl,-Map,$(BIN_DIR)/$(OUTPUT).map,--cref,--gc-sections $(LIB_DIRS)


ASM_OBJS     := $(patsubst %.s,%.o,  $(notdir $(ASM_SRCS)))
C_OBJS       := $(patsubst %.c,%.o,  $(notdir $(C_SRCS)))
CPP_OBJS     := $(patsubst %.cpp,%.o,$(notdir $(CPP_SRCS)))

TARGET_HEX   := $(BIN_DIR)/$(OUTPUT).hex
TARGET_ELF   := $(BIN_DIR)/$(OUTPUT).elf
ASM_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(ASM_OBJS))
C_OBJS_EXT   := $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT   := $(patsubst %.o, %.d, $(C_OBJS_EXT))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o, %.d, $(CPP_OBJS_EXT))

# create $(BIN_DIR) if it does not exist
ifeq ("$(wildcard $(BIN_DIR))","")
$(shell $(MKDIR) $(BIN_DIR))
endif

#-----------------------------------------------------------------------------
# rules
#

all: $(TARGET_HEX)
#all: $(TARGET_ELF)

$(TARGET_HEX): $(TARGET_ELF)
	$(BIN) -O ihex $< $@

$(TARGET_ELF) : $(ASM_OBJS_EXT) $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(CPP) $(CPPFLAGS) -c $(QPCPP)/include/qstamp.cpp -o $(BIN_DIR)/qstamp.o
	$(LINK) $(LINKFLAGS) -o $@ $^ $(BIN_DIR)/qstamp.o $(LIBS)

$(BIN_DIR)/%.d : %.c
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $< > $@

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $< > $@

$(BIN_DIR)/%.o : %.s
	$(AS) $(ASFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

# include dependency files only if our goal depends on their existence
ifneq ($(MAKECMDGOALS),clean)
  ifneq ($(MAKECMDGOALS),show)
-include $(C_DEPS_EXT) $(CPP_DEPS_EXT)
  endif
endif


.PHONY : clean
clean:
	-$(RM) $(BIN_DIR)/*.o \
	$(BIN_DIR)/*.d \
	$(BIN_DIR)/*.hex \
	$(BIN_DIR)/*.elf \
	$(BIN_DIR)/*.map
	
show:
	@echo PROJECT = $(PROJECT)
	@echo CONF = $(CONF)
	@echo DEFINES = $(DEFINES)
	@echo ASM_FPU = $(ASM_FPU)
	@echo ASM_SRCS = $(ASM_SRCS)
	@echo C_SRCS = $(C_SRCS)
	@echo CPP_SRCS = $(CPP_SRCS)
	@echo ASM_OBJS_EXT = $(ASM_OBJS_EXT)
	@echo C_OBJS_EXT = $(C_OBJS_EXT)
	@echo C_DEPS_EXT = $(C_DEPS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
	@echo TARGET_ELF = $(TARGET_ELF)
