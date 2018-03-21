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

# Use the BUFFALO submodule if it exists and BUFFALO_DIR is not set
BUFFALO_SUBMODULE    := $(CURDIR)/buffalo
ifneq ($(wildcard $(BUFFALO_SUBMODULE)/Makefile),)
  BUFFALO_DIR        ?= $(BUFFALO_SUBMODULE)
else
  BUFFALO_DIR        ?= $(shell dirname `pwd`)/buffalo
endif

# buffalo
APPLICATION_DIR    := $(BUFFALO_DIR)
APPLICATION_NAME   := buffalo
include            $(FRAMEWORK_DIR)/app.mk

EXTERNAL_FLAGS	    += -Wl,-rpath,$(OPENMC_DIR)/lib -L$(OPENMC_DIR)/lib -lopenmc
ifneq "$(ENABLE_NEK_COUPLING)" "false"
  EXTERNAL_FLAGS    += -Wl,-rpath,$(MOON_DIR)/lib -L$(MOON_DIR)/lib -lopenmc
endif

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := okapi
BUILD_EXEC         := yes
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk
