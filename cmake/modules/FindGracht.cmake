# FindGracht - attempts to locate the gracht protocol library.
#
# This module defines the following variables (on success):
# GRACHT_INCLUDE_DIRS - where to find gracht/client.h
# GRACHT_LIBRARY - the name of the library
# GRACHT_FOUND - if the library was successfully located
#
# It is trying a few standard installation locations, but can be customized
# with the following variables:
# GRACHT_ROOT - root directory of a gracht installation
# Headers are expected to be found in either:
# <GRACHT_ROOT>/gracht/client.h OR
# <GRACHT_ROOT>/include/gracht/client.h
# This variable can either be a cmake or environment
# variable. Note however that changing the value
# of the environment varible will NOT result in
# re-running the header search and therefore NOT
# adjust the variables set by this module.
#=============================================================================
# default search dirs

SET(_gracht_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes" )
set(_gracht_LIB_SEARCH_DIRS
"/usr/lib"
"/usr/local/lib"
"${CMAKE_SOURCE_DIR}/lib" )

# check environment variable
SET(_gracht_ENV_ROOT_DIR "$ENV{GRACHT_ROOT}")
IF(NOT GRACHT_ROOT AND _gracht_ENV_ROOT_DIR)
	SET(GRACHT_ROOT "${_gracht_ENV_ROOT_DIR}")
ENDIF(NOT GRACHT_ROOT AND _gracht_ENV_ROOT_DIR)

# put user specified location at beginning of search
IF(GRACHT_ROOT)
	list( INSERT _gracht_HEADER_SEARCH_DIRS 0 "${GRACHT_ROOT}/include" )
	list( INSERT _gracht_LIB_SEARCH_DIRS 0 "${GRACHT_ROOT}/lib" )
ENDIF(GRACHT_ROOT)

# Search for the header
FIND_PATH(GRACHT_INCLUDE_DIR "gracht/client.h"
PATHS ${_gracht_HEADER_SEARCH_DIRS} )

# Search for the library
FIND_LIBRARY(GRACHT_LIBRARY NAMES libgracht gracht
PATHS ${_gracht_LIB_SEARCH_DIRS} )
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gracht DEFAULT_MSG
GRACHT_LIBRARY GRACHT_INCLUDE_DIR)
