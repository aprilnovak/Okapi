###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
#
###############################################################################
# Environment variable indicating if we should link to the MOON (Moose-wrapped
# Nek5000) library and compile Okapi source files that contain calls to Nek5000
# subroutines. Setting this to 'false' indicates that Okapi will be used only
# for OpenMC-MOOSE coupling. If you change the value of this variable, make sure
# to run 'make clean' before re-compiling.
ENABLE_NEK_COUPLING := false

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

ifdef BUFFALO_DIR
# buffalo
APPLICATION_DIR    := $(BUFFALO_DIR)
APPLICATION_NAME   := buffalo
include            $(FRAMEWORK_DIR)/app.mk
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

OPENMC_BUILD_DIR := $(OPENMC_DIR)/build

ifeq ($(UNAME), Linux)
  OPENMC_LIB = $(OPENMC_BUILD_DIR)/lib/libopenmc.so
else
  OPENMC_LIB = $(OPENMC_BUILD_DIR)/lib/libopenmc.dylib
endif

$(OPENMC_BUILD_DIR):
	mkdir $(OPENMC_BUILD_DIR)

$(OPENMC_LIB): $(OPENMC_BUILD_DIR)
	rm -rf $(OPENMC_BUILD_DIR)
	mkdir $(OPENMC_BUILD_DIR)
	cd $(OPENMC_BUILD_DIR) && cmake $(OPENMC_DIR) && $(MAKE)

ADDITIONAL_DEPEND_LIBS += $(OPENMC_LIB)

################################## GET FLAGS RIGHT ####################################

ADDITIONAL_LIBS	    += -Wl,-rpath,$(OPENMC_BUILD_DIR)/lib -L$(OPENMC_BUILD_DIR)/lib -lopenmc
ifneq "$(ENABLE_NEK_COUPLING)" "false"
  ADDITIONAL_LIBS    += -Wl,-rpath,$(MOON_DIR)/lib -L$(MOON_DIR)/lib -lmoon
endif
ifdef BUFFALO_DIR
ADDITIONAL_CPPFLAGS += -DENABLE_BUFFALO_COUPLING
endif

################################## OKAPI ####################################

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := okapi
BUILD_EXEC         := yes
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk
