###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Directory Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
# BUFFALO_DIR      - Root directory of the BUFFALO project
# MOON_DIR         - Root directory of the MOON project
#
###############################################################################

# Use the MOOSE submodule if it exists and MOOSE_DIR is not set
MOOSE_SUBMODULE    := $(CURDIR)/moose
ifneq ($(wildcard $(MOOSE_SUBMODULE)/framework/Makefile),)
  MOOSE_DIR        ?= $(MOOSE_SUBMODULE)
else
  MOOSE_DIR        ?= $(shell dirname `pwd`)/moose
endif

# framework
FRAMEWORK_DIR      := $(MOOSE_DIR)/framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
ALL_MODULES         := no

include $(MOOSE_DIR)/modules/modules.mk

################################## BUFFALO ####################################
# Local variable indicating if we should link to the BUFFALO (BISON stand-in)
# library. If you change the value of this variable, make sure to run 'make clean'
# before re-compiling.
ENABLE_BUFFALO_COUPLING := true

ifdef ENABLE_BUFFALO_COUPLING
# Use a BUFFALO directory if its on the same level as Okapi and BUFFALO_DIR is not set
ifneq ($(wildcard $(shell dirname `pwd`)/Buffalo/Makefile),)
  BUFFALO_DIR        ?= $(shell dirname `pwd`)/Buffalo
endif

ifdef BUFFALO_DIR
APPLICATION_DIR    := $(BUFFALO_DIR)
APPLICATION_NAME   := buffalo
include            $(FRAMEWORK_DIR)/app.mk
endif
endif

################################## MOON ####################################
# Local variable indicating if we should link to the MOON (Moose-wrapped
# Nek5000) library and compile Okapi source files that contain calls to Nek5000
# subroutines. Setting this to 'false' indicates that Okapi will be used only
# for OpenMC-MOOSE coupling. If you change the value of this variable, make sure
# to run 'make clean' before re-compiling.
ENABLE_NEK_COUPLING := false

ifdef ENABLE_NEK_COUPLING
# Use a MOON directory if its on the same level as Okapi and MOON_DIR is not set
ifneq ($(wildcard $(shell dirname `pwd`)/moon/Makefile),)
  MOON_DIR        ?= $(shell dirname `pwd`)/moon
endif

ifdef MOON_DIR
APPLICATION_DIR    := $(MOON_DIR)
APPLICATION_NAME   := moon
include            $(FRAMEWORK_DIR)/app.mk
endif
endif

################################## OPENMC ####################################
ifndef HDF5_ROOT
$(error The HDF5_ROOT environment varible must be set in order to compile openmc)
endif

# Use the OPENMC submodule if it exists and OPENMC_DIR is not set
OPENMC_SUBMODULE    := $(CURDIR)/openmc
ifneq ($(wildcard $(OPENMC_SUBMODULE)/CMakeLists.txt),)
  OPENMC_DIR        ?= $(OPENMC_SUBMODULE)
else
  OPENMC_DIR        ?= $(shell dirname `pwd`)/openmc
endif

ifeq ($(METHOD),dbg)
OPENMC_BUILD_DIR := $(OPENMC_DIR)/build-dbg
CMAKE := cmake -Ddebug=ON
else
OPENMC_BUILD_DIR := $(OPENMC_DIR)/build
CMAKE := cmake
endif

ifeq ($(UNAME), Linux)
  OPENMC_LIB = $(OPENMC_BUILD_DIR)/lib/libopenmc.so
else
  OPENMC_LIB = $(OPENMC_BUILD_DIR)/lib/libopenmc.dylib
endif

$(OPENMC_BUILD_DIR)/Makefile:
	mkdir -p $(OPENMC_BUILD_DIR)
	cd $(OPENMC_BUILD_DIR) && $(CMAKE) $(OPENMC_DIR)

$(OPENMC_LIB): $(OPENMC_BUILD_DIR)/Makefile
	cd $(OPENMC_BUILD_DIR) && $(MAKE) --no-print-directory

ADDITIONAL_DEPEND_LIBS += $(OPENMC_LIB)

################################## GET FLAGS RIGHT ####################################

ADDITIONAL_LIBS	    += -Wl,-rpath,$(OPENMC_BUILD_DIR)/lib -L$(OPENMC_BUILD_DIR)/lib -lopenmc
ifneq ($(and $(ENABLE_BUFFALO_COUPLING),$(BUFFALO_DIR)),)
ADDITIONAL_CPPFLAGS += -DENABLE_BUFFALO_COUPLING
endif
ifneq ($(and $(ENABLE_NEK_COUPLING),$(MOON_DIR)),)
ADDITIONAL_CPPFLAGS += -DENABLE_NEK_COUPLING
endif

################################## OKAPI ####################################

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := okapi
BUILD_EXEC         := yes
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk
