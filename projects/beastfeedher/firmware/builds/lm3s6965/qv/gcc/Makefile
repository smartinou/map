# ***************************************************************************************
#
# Makefile
#
# ***************************************************************************************
#
#     Copyright (c) 2017-2020, Pleora Technologies Inc., All rights reserved.
#
# ***************************************************************************************
#
# File Name....: Makefile
#
# Description..: Makefile for generation of project's executable.
#
# ***************************************************************************************

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Makefile variables
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.PHONY: help
help:
	@echo Building configurations: Debug $(default), Release, and Spy
	@echo 
	@echo Targets:
	@echo 'make          - Debug (default) configuration.'
	@echo 'make CONF=rel - Release configuration.'
	@echo 'make CONF=spy - Spy configuration.'
	@echo
	@echo Cleaning configurations: Debug $(default), Release, and Spy
	@echo 
	@echo Targets:
	@echo 'make clean         - Debug (default) configuration.'
	@echo 'make CONF=rel clean - Release configuration.'
	@echo 'make CONF=spy clean - Spy configuration.'
	@echo
	@echo 'make *_clean - Cleans target and intermediate files.'


# NOTE:
# To use this Makefile on Windows, you will need the GNU make utility, which
# is included in the Qtools collection for Windows, see:
#    http://sourceforge.net/projects/qpc/files/Qtools/
#

# Project name
PROJECT := pfpp

# Directories/paths.
REPO_ROOT_DIR := ../../../../../../..
FIRMWARE_PATH := $(REPO_ROOT_DIR)/firmware
PROJECT_ROOT_DIR := $(REPO_ROOT_DIR)/projects/beastfeedher
PROJECT_FW_DIR   := $(PROJECT_ROOT_DIR)/firmware

# LwIP library.
LWIPDIR := ../../../../../../../firmware/3rdparty/lwip-2.1.2/src
include $(LWIPDIR)/Filelists.mk


# Location of the QP/C++ framework (if not provided in an environemnt var.)
ifeq ($(QPCPP),)
QPCPP := $(FIRMWARE_PATH)/3rdparty/qp/qpcpp_6_5_1
endif

# QP port used in this project
QP_PORT_DIR := $(QPCPP)/ports/arm-cm/qv/gnu

# Stellaris/TI driver library.
ifeq ($(STELLARIS_LIB),)
STELLARIS_LIB := $(FIRMWARE_PATH)/3rdparty/StellarisWare/
endif

# CMSIS library.
CMSIS_LIB := $(FIRMWARE_PATH)/3rdparty/CMSIS_LM3S-32/CMSIS_LM3S

# List of all source directories used by this project.
VPATH = \
        ../../ \
        $(PROJECT_FW_DIR)/src \
        $(PROJECT_FW_DIR)/src/models/codegen \
        $(PROJECT_FW_DIR)/src/lwip_port/arch \
        $(PROJECT_FW_DIR)/src/lwip_port/netif/lm3s \
        $(FIRMWARE_PATH)/corelink/src \
        $(FIRMWARE_PATH)/drivers/src \
        $(FIRMWARE_PATH)/qp_ao/codegen \
        $(FIRMWARE_PATH)/qp_ao/src \
        $(FIRMWARE_PATH)/utils/date \
        $(FIRMWARE_PATH)/utils/db \
        $(FIRMWARE_PATH)/utils/net \
        $(FIRMWARE_PATH)/utils/src \
        $(FIRMWARE_PATH)/utils/time \
        $(QPCPP)/src/qf \
        $(QPCPP)/src/qs \
        $(QPCPP)/src/qv \
        $(QP_PORT_DIR) \
        $(CMSIS_LIB)/lm3s \
        $(CMSIS_LIB)/lm3s/GCC \
        $(CMSIS_LIB)/core \
        $(LWIPDIR)/core \
        $(LWIPDIR)/core/ipv4 \
        $(LWIPDIR)/api \
        $(LWIPDIR)/netif \
        $(LWIPDIR)/apps/http \
        $(LWIPDIR)/apps/mqtt \
        $(LWIPDIR)/apps/snmp \
        $(LWIPDIR)/apps/sntp \
        $(FAT_FS_SRC_DIR)

# List of all include directories needed by this project.
INCLUDES  = \
        -I. \
        -I$(PROJECT_ROOT_DIR)/firmware/src \
        -I$(PROJECT_ROOT_DIR)/firmware/src/ble/apps/ble_pfpp/src \
        -I$(PROJECT_ROOT_DIR)/firmware/src/lwip_port \
        -I$(PROJECT_ROOT_DIR)/firmware/src/models/codegen \
        -I$(PROJECT_ROOT_DIR)/firmware/builds/lm3s6965 \
        -I$(FIRMWARE_PATH)/corelink/inc \
        -I$(FIRMWARE_PATH)/drivers/inc \
        -I$(FIRMWARE_PATH)/qp_ao/inc \
        -I$(FIRMWARE_PATH)/qp_ao/codegen \
        -I$(FIRMWARE_PATH)/utils \
        -I$(STELLARIS_LIB) \
        -I$(QPCPP)/include \
        -I$(QPCPP)/src \
        -I$(QP_PORT_DIR) \
        -I$(CMSIS_LIB)/lm3s \
        -I$(CMSIS_LIB)/core \
        -I$(CMSIS_LIB) \
        -I$(LWIPDIR)/include \
        -I$(FAT_FS_INC_DIR)

# Files
# Assembler source files.
ASM_SRCS :=



FAT_FS_DIR := $(FIRMWARE_PATH)/3rdparty/ff13a
FAT_FS_SRC_DIR := $(FAT_FS_DIR)/source
FAT_FS_INC_DIR := $(FAT_FS_SRC_DIR)

FAT_FS_SRCS := \
        ff.c

# C source files.
C_SRCS := \
	startup_lm3s6965.c \
	system_lm3s.c \
	uartstdio.c \
	$(SNTPFILES) \
	$(COREFILES) \
	$(CORE4FILES) \
	$(APIFILES) \
	$(LWIPDIR)/netif/ethernet.c \
	$(HTTPFILES) \
    $(MQTTFILES) \
    $(FAT_FS_SRCS)


# C++ source files.
CPP_SRCS := \
        App.cpp \
        BLE.cpp \
        Main.cpp \
        BLE_AOs.cpp \
        BSP.cpp \
        Button.cpp \
        CalendarRec.cpp \
        CoreLinkPeripheralDev.cpp \
        Date.cpp \
        DBRec.cpp \
        Display_AOs.cpp \
        DS3234.cpp \
        EthernetAddress.cpp \
        EthDrv.cpp \
        FatFSDisk.cpp \
        FeedCfgRec.cpp \
        GPIO.cpp \
        IPAddress.cpp \
        Limit.cpp \
        Logging_AOs.cpp \
        Logger.cpp \
        LogLevel.cpp \
        LwIP_AOs.cpp \
        LwIPDrv.cpp \
        Month.cpp \
        Net.cpp \
        NetIFRec.cpp \
        PFPP_AOs.cpp \
        IRTCC.cpp \
        RTCC_AOs.cpp \
        SDC.cpp \
        SSD1329.cpp \
        SPI.cpp \
        SPISlaveCfg.cpp \
        TB6612.cpp \
        Time.cpp \
        Weekday.cpp

#        Logging_Events.cpp \
#        eth_driver.cpp \
#        RTCC_Events.cpp \

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
        qv.cpp \
        qv_port.cpp

QP_ASMS :=
QS_SRCS := \
        qs.cpp \
        qs_rx.cpp \
        qs_fp.cpp


OUTPUT    := $(PROJECT)
LD_SCRIPT := lm3s6965_gcc.ld

LIB_DIRS  := -L$(STELLARIS_LIB)/driverlib/gcc-cm3
LIBS      := -ldriver-cm3

# defines
# [MG] SEE IF CAN BE MOVED SPECIFICALLY TO QP FILES.
DEFINES   := -DQ_NASSERT
DEFINES   += -D__STARTUP_CLEAR_BSS
DEFINES   += -DQ_EVT_CTOR
DEFINES   += -DQF_ACTIVE_STOP
DEFINES   += -DNET_SUPPORT
DEFINES   += -DUSE_STELLARISWARE

# Override heap value (see startup_lm3s6965.c)
# This does not appear in the .bss or .data section.
DEFINES   += -D__HEAP_SIZE=0x4000

# ARM CPU, FPU, and Float-ABI types
ARM_CPU   := -mcpu=cortex-m3
ARM_FPU   :=
FLOAT_ABI :=


# GNU-ARM toolset (NOTE: You need to adjust to your machine)
# see http://gnutoolchains.com/arm-eabi/
ifeq ($(GNU_ARM),)
#GNU_ARM = C:/tools/gnu_arm-eabi
#GNU_ARM = C:/apps/GNU\ Tools\ ARM\ Embedded/4.9\ 2015q3
#GNU_ARM = C:/apps/GNU\ Tools\ ARM\ Embedded/6\ 2017-q2-update
#GNU_ARM = C:/apps/GNU\ Tools\ ARM\ Embedded/8\ 2019-q3-update
#GNU_ARM = C:/apps/GNU\ Tools\ ARM\ Embedded/9\ 2019-q4-major
GNU_ARM = C:/apps/GNU\ Arm\ Embedded\ Toolchain/10\ 2021.10
endif

# make sure that the GNU-ARM toolset exists...
ifeq ("$(wildcard $(GNU_ARM))","")
$(error GNU_ARM toolset not found. Please adjust the Makefile)
endif

CC    := $(GNU_ARM)/bin/arm-none-eabi-gcc
CPP   := $(GNU_ARM)/bin/arm-none-eabi-g++
AR    := $(GNU_ARM)/bin/arm-none-eabi-ar
AS    := $(GNU_ARM)/bin/arm-none-eabi-as
LINK  := $(GNU_ARM)/bin/arm-none-eabi-g++
BIN   := $(GNU_ARM)/bin/arm-none-eabi-objcopy
SIZE  := $(GNU_ARM)/bin/arm-none-eabi-size

# Basic utilities (included in Windows bin tools), see:
MKDIR := mkdir
RM    := rm


# Build options for various configurations for ARM Cortex-M4F

# Combine all the sources...
#VPATH    += $(QPCPP)/3rd_party/gnu_cpp
VPATH    += $(QPCPP)/3rd_party/gnu
#CPP_SRCS += $(QP_SRCS) mini_cpp.cpp
CPP_SRCS += $(QP_SRCS)
ASM_SRCS += $(QP_ASMS)

# Add the pre-defined symbol __FPU_PRESENT if needed...
ifneq (,$(ARM_FPU))
DEFINES += -D__FPU_PRESENT
ASM_FPU += -defsym=__FPU_PRESENT=1
endif


# Release configuration.
ifeq (rel, $(CONF))

BIN_DIR := rel

ASFLAGS = $(ARM_CPU) $(ASM_FPU)

CFLAGS = $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) \
    -mthumb -Wall \
	-ffunction-sections -fdata-sections \
	-O2

CPPFLAGS = $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) \
    -mthumb -Wall \
	-ffunction-sections -fdata-sections -fno-rtti -fno-exceptions \
	-std=c++14 \
	-O2

DEFINES += -DNDEBUG

else ifeq (spy, $(CONF))
# Spy configuration.

BIN_DIR := spy

CPP_SRCS += $(QS_SRCS)

ASFLAGS = -g $(ARM_CPU) $(ASM_FPU)

CFLAGS = -g $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) \
    -mthumb -Wall \
	-ffunction-sections -fdata-sections \
	-O

CPPFLAGS = -g $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) \
    -mthumb -Wall \
	-ffunction-sections -fdata-sections -fno-rtti -fno-exceptions \
	-std=c++14 \
	-O

DEFINES += -DQ_SPY

else
# Default Debug configuration.
BIN_DIR := dbg

ASFLAGS = -g $(ARM_CPU) $(ASM_FPU)

CFLAGS = -g $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) \
    -mthumb -Wall \
	-ffunction-sections -fdata-sections \
	-O

CPPFLAGS = -g $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) \
    -mthumb -Wall \
	-ffunction-sections -fdata-sections -fno-rtti -fno-exceptions \
	-std=c++14 \
	-O

endif


LINKFLAGS = -T$(LD_SCRIPT) $(ARM_CPU) $(ARM_FPU) $(FLOAT_ABI) \
	-mthumb \
	--specs=nosys.specs \
	-Wl,-Map,$(BIN_DIR)/$(OUTPUT).map,--cref,--gc-sections $(LIB_DIRS)


ASM_OBJS     := $(patsubst %.s,%.o,  $(notdir $(ASM_SRCS)))
C_OBJS       := $(patsubst %.c,%.o,  $(notdir $(C_SRCS)))
CPP_OBJS     := $(patsubst %.cpp,%.o,$(notdir $(CPP_SRCS)))

TARGET_BIN   := $(BIN_DIR)/$(OUTPUT).bin
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

# Web & HTML variables.
WEB_INPUT_DIR   := $(PROJECT_FW_DIR)/src/html/in
WEB_OUTPUT_DIR  := $(PROJECT_FW_DIR)/src/html/out
MAKE_FSDATA_DIR := $(LWIPDIR)/apps/http/makefsdata

WEB_HTML_FILES := \
  config.shtml \
  index.shtml \
  network.shtml \
  stats.shtml \
  Styles.css

WEB_BIN_FILES := \
  BGround.png

HTML_OUTPUT_FILES := $(foreach FILE, $(WEB_HTML_FILES), $(WEB_OUTPUT_DIR)/$(FILE))

BIN_OUTPUT_FILES := $(foreach FILE, $(WEB_BIN_FILES), $(WEB_OUTPUT_DIR)/$(FILE))

QM_PATH := c:/apps/qm/bin
QM_BIN := $(QM_PATH)/qmc.exe


MODEL_IN_FILES := \
  $(FIRMWARE_PATH)/qp_ao/model/Logging.qm

MODEL_OUT_FILES := \


#FileLogSink_AO.h \
#FileLogSink_AO.cpp \
#RTCC_AOs.cpp \
#Logger.h \
#Logger.cpp \
#LogEvt.h \
#LogEvt.cpp

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Targets.
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

all: show version generate_fsdata $(TARGET_BIN)
#all: $(TARGET_ELF)

.PHONY: version
version:
	./make_version.sh


remove_white_space: $(HTML_OUTPUT_FILES)

copy_binary_files: $(BIN_OUTPUT_FILES)

generate_fsdata: remove_white_space copy_binary_files
	cd $(PROJECT_ROOT_DIR)/firmware/src/html && \
  $(abspath $(MAKE_FSDATA_DIR))/makefsdata && \
  mv fsdata.c ../fsdata_custom.c

# Remove leading white spaces and tabs from files.
$(HTML_OUTPUT_FILES): $(WEB_INPUT_DIR)/$(notdir $@)
	mkdir -p $(WEB_OUTPUT_DIR)
	cat $(WEB_INPUT_DIR)/$(notdir $@) | sed 's/^[ \t]*//;s/[ \t]*$$//' > $@

$(BIN_OUTPUT_FILES):
	cp $(WEB_INPUT_DIR)/$(notdir $@) $@

$(MODEL_OUT_FILES): $(MODEL_IN_FILES)
	$(QM_BIN) $?



$(TARGET_BIN): $(TARGET_ELF)
	$(BIN) -O binary $< $@
	$(SIZE) $<

$(TARGET_ELF) : $(ASM_OBJS_EXT) $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(CPP) $(CPPFLAGS) -c $(QPCPP)/include/qstamp.cpp -o $(BIN_DIR)/qstamp.o
	$(LINK) $(LINKFLAGS) -o $@ $^ $(BIN_DIR)/qstamp.o $(LIBS)

$(BIN_DIR)/%.d : %.c
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $(INCLUDES) $(DEFINES) $< > $@

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< > $@

$(BIN_DIR)/%.o : %.s
	$(AS) $(ASFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

# Include dependency files only if our goal depends on their existence
ifneq ($(MAKECMDGOALS),clean)
  ifneq ($(MAKECMDGOALS),show)
-include $(C_DEPS_EXT) $(CPP_DEPS_EXT)
  endif
endif


.PHONY: clean
clean:
	-$(RM) $(BIN_DIR)/*.o \
        $(BIN_DIR)/*.d \
        $(BIN_DIR)/*.bin \
        $(BIN_DIR)/*.elf \
        $(BIN_DIR)/*.map
	-$(RM) -rf $(WEB_OUTPUT_DIR)

show:
	@echo PROJECT = $(PROJECT)
	@echo CONF = $(CONF)
	@echo DEFINES = $(DEFINES)
	@echo CFLAGS = $(CFLAGS)
	@echo CPPFLAGS = $(CPPFLAGS)
	@echo ASM_FPU = $(ASM_FPU)
	@echo ASM_SRCS = $(ASM_SRCS)
	@echo C_SRCS = $(C_SRCS)
	@echo CPP_SRCS = $(CPP_SRCS)
	@echo ASM_OBJS_EXT = $(ASM_OBJS_EXT)
	@echo C_OBJS_EXT = $(C_OBJS_EXT)
	@echo C_DEPS_EXT = $(C_DEPS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
	@echo TARGET_ELF = $(TARGET_ELF)
	@echo TARGET_BIN = $(TARGET_BIN)
