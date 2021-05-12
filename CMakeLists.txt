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
set(GRACHT_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GRACHT_INSTALL_DIR})

ExternalProject_Add(gracht
  PREFIX ${GRACHT_PREFIX}
  GIT_REPOSITORY https://github.com/Meulengracht/libgracht.git
  GIT_TAG main
  INSTALL_DIR ${GRACHT_INSTALL_DIR}
  CMAKE_ARGS ${GRACHT_CMAKE_ARGS}
)

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

add_subdirectory (libs/asgaard)
add_subdirectory (libs/glad)

add_subdirectory (core)
