cmake_minimum_required (VERSION 3.12)

project (Vioarr C CXX)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

include (CheckIncludeFiles)
check_include_files (threads.h HAVE_C11_THREADS)
check_include_files (pthread.h HAVE_PTHREAD)

find_package (GLM REQUIRED)
message (STATUS "Found GRLM in ${GLM_INCLUDE_DIR}")
find_package (GLFW3 REQUIRED)
message (STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package (Python3 REQUIRED)

# checkout dependencies that we need
include(ExternalProject)

set(GRACHT_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gracht")
set(GRACHT_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/gracht")
set(GRACHT_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GRACHT_INSTALL_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})

ExternalProject_Add(gracht_dl
  PREFIX ${GRACHT_PREFIX}
  GIT_REPOSITORY https://github.com/meulengracht/libgracht.git
  GIT_TAG main
  INSTALL_DIR ${GRACHT_INSTALL_DIR}
  CMAKE_ARGS ${GRACHT_CMAKE_ARGS}
  CONFIGURE_HANDLED_BY_BUILD ON
  BUILD_BYPRODUCTS ${GRACHT_INSTALL_DIR}/lib/${CMAKE_SHARED_MODULE_PREFIX}gracht${CMAKE_SHARED_LIBRARY_SUFFIX}
)
add_library(gracht SHARED IMPORTED GLOBAL)
set_target_properties(gracht PROPERTIES IMPORTED_LOCATION ${GRACHT_INSTALL_DIR}/lib/${CMAKE_SHARED_MODULE_PREFIX}gracht${CMAKE_SHARED_LIBRARY_SUFFIX})

set(LIBPNG_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libpng")
set(LIBPNG_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/libpng")
set(LIBPNG_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIBPNG_INSTALL_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})

ExternalProject_Add(libpng_dl
  PREFIX ${LIBPNG_PREFIX}
  GIT_REPOSITORY https://github.com/umbrella-c/libpng.git
  GIT_TAG main
  INSTALL_DIR ${LIBPNG_INSTALL_DIR}
  CMAKE_ARGS ${LIBPNG_CMAKE_ARGS}
  CONFIGURE_HANDLED_BY_BUILD ON
  BUILD_BYPRODUCTS ${LIBPNG_INSTALL_DIR}/lib/${CMAKE_SHARED_MODULE_PREFIX}png${CMAKE_SHARED_LIBRARY_SUFFIX}
)
add_library(libpng SHARED IMPORTED GLOBAL)
set_target_properties(libpng PROPERTIES IMPORTED_LOCATION ${LIBPNG_INSTALL_DIR}/lib/${CMAKE_SHARED_MODULE_PREFIX}png${CMAKE_SHARED_LIBRARY_SUFFIX})

set(LIBZIP_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libzip")
set(LIBZIP_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/libzip")
set(LIBZIP_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIBZIP_INSTALL_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})

ExternalProject_Add(libzip_dl
  PREFIX ${LIBZIP_PREFIX}
  GIT_REPOSITORY https://github.com/umbrella-c/libzip.git
  GIT_TAG main
  INSTALL_DIR ${LIBZIP_INSTALL_DIR}
  CMAKE_ARGS ${LIBZIP_CMAKE_ARGS}
  CONFIGURE_HANDLED_BY_BUILD ON
  BUILD_BYPRODUCTS 
    ${LIBZIP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}bzip2${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${LIBZIP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lzma${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${LIBZIP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zip${CMAKE_STATIC_LIBRARY_SUFFIX}
)
add_library(libbzip2 STATIC IMPORTED GLOBAL)
set_target_properties(libbzip2 PROPERTIES IMPORTED_LOCATION ${LIBZIP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}bzip2${CMAKE_STATIC_LIBRARY_SUFFIX})
add_library(liblzma STATIC IMPORTED GLOBAL)
set_target_properties(liblzma PROPERTIES IMPORTED_LOCATION ${LIBZIP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lzma${CMAKE_STATIC_LIBRARY_SUFFIX})
add_library(libzip STATIC IMPORTED GLOBAL)
set_target_properties(libzip PROPERTIES IMPORTED_LOCATION ${LIBZIP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zip${CMAKE_STATIC_LIBRARY_SUFFIX})

if (WIN32)
    set (LIBS glfw3 opengl32 freetype)
elseif (UNIX AND NOT APPLE)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    find_package (OpenGL REQUIRED)
    add_definitions (${OPENGL_DEFINITIONS})
    find_package (X11 REQUIRED)

    # the order of link libraries here is important
    set (LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype)
    set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif (APPLE)
    include_directories (/System/Library/Frameworks)
    find_library (COCOA_LIBRARY Cocoa)
    find_library (OpenGL_LIBRARY OpenGL)
    find_library (IOKit_LIBRARY IOKit)
    find_library (CoreVideo_LIBRARY CoreVideo)
    mark_as_advanced (COCOA_LIBRARY OpenGL_LIBRARY)
    set (APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    set (APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})
    set (LIBS ${LIBS} ${APPLE_LIBS})
elseif (MOLLENOS)
    include_directories($ENV{VALI_APPLICATION_PATH}/include)
    include_directories($ENV{VALI_DDK_PATH}/include)

    link_directories($ENV{VALI_APPLICATION_PATH}/lib)
    link_directories($ENV{VALI_DDK_PATH}/lib)

endif ()

add_subdirectory (protocols)

add_subdirectory (core)

add_subdirectory (libs/freetype)
add_subdirectory (libs/asgaard)
add_subdirectory (libs/glad)
