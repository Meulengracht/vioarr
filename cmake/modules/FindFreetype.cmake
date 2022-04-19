# FindFreetype - attempts to locate the freetype rendering library.
#
# This module defines the following variables (on success):
# FREETYPE_INCLUDE_DIRS - where to find ft2build.h
# FREETYPE_LIBRARY - the name of the library
# FREETYPE_FOUND - if the library was successfully located
#
# It is trying a few standard installation locations, but can be customized
# with the following variables:
# FREETYPE_ROOT - root directory of a freetype installation
# Headers are expected to be found in:
# <FREETYPE_ROOT>/include/ft2build.h
# This variable can either be a cmake or environment
# variable. Note however that changing the value
# of the environment varible will NOT result in
# re-running the header search and therefore NOT
# adjust the variables set by this module.
#=============================================================================
# default search dirs

SET(_freetype_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes" )
set(_freetype_LIB_SEARCH_DIRS
"/usr/lib"
"/usr/local/lib"
"${CMAKE_SOURCE_DIR}/lib" )

# check environment variable
SET(_freetype_ENV_ROOT_DIR "$ENV{FREETYPE_ROOT}")
IF(NOT FREETYPE_ROOT AND _freetype_ENV_ROOT_DIR)
	SET(FREETYPE_ROOT "${_freetype_ENV_ROOT_DIR}")
ENDIF(NOT FREETYPE_ROOT AND _freetype_ENV_ROOT_DIR)

# put user specified location at beginning of search
IF(FREETYPE_ROOT)
	list( INSERT _freetype_HEADER_SEARCH_DIRS 0 "${FREETYPE_ROOT}/include" )
	list( INSERT _freetype_LIB_SEARCH_DIRS 0 "${FREETYPE_ROOT}/lib" )
ENDIF(FREETYPE_ROOT)

# Search for the header
FIND_PATH(FREETYPE_INCLUDE_DIR "ft2build.h"
PATHS ${_freetype_HEADER_SEARCH_DIRS} )

# Search for the library
FIND_LIBRARY(FREETYPE_LIBRARY NAMES libfreetype freetype
PATHS ${_freetype_LIB_SEARCH_DIRS} )
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype DEFAULT_MSG
FREETYPE_LIBRARY FREETYPE_INCLUDE_DIR)
