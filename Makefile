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
# subroutines. This variable must NOT be set to anything in order to ignore all
# Nek-related source files and tests. Setting ENABLE_NEK_COUPLING to ANY value
# will allow linking to Nek5000 routines.
ENABLE_NEK_COUPLING :=

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
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).

ALL_MODULES         := no

CHEMICAL_REACTIONS  := no
CONTACT             := no
FLUID_PROPERTIES    := no
HEAT_CONDUCTION     := no
MISC                := no
NAVIER_STOKES       := no
PHASE_FIELD         := no
RDG                 := no
RICHARDS            := no
SOLID_MECHANICS     := no
STOCHASTIC_TOOLS    := no
TENSOR_MECHANICS    := no
WATER_STEAM_EOS     := no
XFEM                := no
POROUS_FLOW         := no

include $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := okapi
BUILD_EXEC         := yes
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here - set the MOON (MOOSE-
# wrapped Nek5000) library path if coupling is enabled.

ifndef ENABLE_NEK_COUPLING
  NEK5000_COUPLING_LIBRARY ?=
else
  NEK5000_COUPLING_LIBRARY ?= $(MOON_DIR)/lib/libmoon-dbg.so
endif

ADDITIONAL_LIBS := /homes/anovak/projects/openmc/build/lib/libopenmc.so $(NEK5000_COUPLING_LIBRARY)
