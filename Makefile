# this is the root of all projects anywhere...
# well anywhere that relies on the SACK system...
#TOP_LEVEL:=1

# can set options here I guess...
#__NO_SQL__=1

# __NO_SQL__=1 can be defined to eliminate SQL depandnacies (unixodbc, sqlite)
#export __NO_SQL__
# __NO_OPTIONS__=1 can be defined to remove option gui dependancy
#export __NO_OPTIONS__

# comment this out to build sqlite into sack.
#__EXTERNAL_SQLITE__=1 $(warning this needs some work... like actually loading/filling the interface)

# PSI Library tests - palette, listbox, menu, etc...
BUILD_TESTS=1
export BUILD_TESTS

PROJECTS+=makefiles 
#src data

# only one that should define this...
# any sub-project which includes makefile.projects
# should not perform master distclean aspects.
MASTER_LEVEL_DISTCLEAN=1
export MASTER_LEVEL_DISTCLEAN

ifndef SACK_BASE
$(warning SACK_BASE not defined, setting to "$(CURDIR)")
SACK_BASE:=$(CURDIR)
export SACK_BASE
endif

include $(SACK_BASE)/Makefile.bag


