# FindZIP - attempts to locate the zip library.
#
# This module defines the following variables (on success):
# ZIP_INCLUDE_DIRS - where to find libzip/ZipFile.h
# ZIP_LIBRARY - the name of the libraries
# ZIP_FOUND - if the library was successfully located
#
# It is trying a few standard installation locations, but can be customized
# with the following variables:
# ZIP_ROOT - root directory of a libzip installation
# Headers are expected to be found in:
# <ZIP_ROOT>/include/libzip/ZipFile.h
# This variable can either be a cmake or environment
# variable. Note however that changing the value
# of the environment varible will NOT result in
# re-running the header search and therefore NOT
# adjust the variables set by this module.
#=============================================================================
# default search dirs

SET(_zip_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes" )
set(_zip_LIB_SEARCH_DIRS
"/usr/lib"
"/usr/local/lib"
"${CMAKE_SOURCE_DIR}/lib" )

# check environment variable
SET(_zip_ENV_ROOT_DIR "$ENV{ZIP_ROOT}")
IF(NOT ZIP_ROOT AND _zip_ENV_ROOT_DIR)
	SET(ZIP_ROOT "${_zip_ENV_ROOT_DIR}")
ENDIF(NOT ZIP_ROOT AND _zip_ENV_ROOT_DIR)

# put user specified location at beginning of search
IF(ZIP_ROOT)
	list( INSERT _zip_HEADER_SEARCH_DIRS 0 "${ZIP_ROOT}/include" )
	list( INSERT _zip_LIB_SEARCH_DIRS 0 "${ZIP_ROOT}/lib" )
ENDIF(ZIP_ROOT)

# Search for the header
FIND_PATH(ZIP_INCLUDE_DIR "libzip/ZipFile.h"
PATHS ${_zip_HEADER_SEARCH_DIRS} )

# Search for the library
FIND_LIBRARY(ZIP_LIBRARY NAMES libzip zip
PATHS ${_zip_LIB_SEARCH_DIRS} )
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Zip DEFAULT_MSG
ZIP_LIBRARY ZIP_INCLUDE_DIR)
