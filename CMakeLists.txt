cmake_minimum_required (VERSION 3.14)

project (Vioarr C CXX)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

include (CheckIncludeFiles)
check_include_files (threads.h HAVE_C11_THREADS)
check_include_files (pthread.h HAVE_PTHREAD)

find_package (Python3 REQUIRED)
find_package (GLM REQUIRED)
message (STATUS "Found GRLM in ${GLM_INCLUDE_DIR}")

if (NOT MOLLENOS)
  find_package (GLFW3 REQUIRED)
  message (STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
endif ()

# checkout dependencies that we need
include(FetchContent)

FetchContent_Declare(
  libgracht
  GIT_REPOSITORY https://github.com/meulengracht/libgracht.git
  GIT_TAG main
)
FetchContent_MakeAvailable(libgracht)

FetchContent_Declare(
  png
  GIT_REPOSITORY https://github.com/umbrella-c/libpng.git
  GIT_TAG main
)
FetchContent_MakeAvailable(png)

FetchContent_Declare(
  zip
  GIT_REPOSITORY https://github.com/umbrella-c/libzip.git
  GIT_TAG main
)

set(LIBZIP_COMPILE_AS_PIC ON CACHE INTERNAL "Build libzip with -fPIC enabled")
FetchContent_MakeAvailable(zip)

if (WIN32)
    set (LIBS glfw3 opengl32 freetype)
elseif (UNIX AND NOT APPLE)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    find_package (OpenGL REQUIRED)
    add_definitions (${OPENGL_DEFINITIONS})
    find_package (X11 REQUIRED)

    # the order of link libraries here is important
    set (LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL m pthread freetype ${CMAKE_DL_LIBS})
    set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -l${CMAKE_DL_LIBS}")
elseif (APPLE)
    include_directories (/System/Library/Frameworks)
    find_library (COCOA_LIBRARY Cocoa)
    find_library (OpenGL_LIBRARY OpenGL)
    find_library (IOKit_LIBRARY IOKit)
    find_library (CoreVideo_LIBRARY CoreVideo)
    mark_as_advanced (COCOA_LIBRARY OpenGL_LIBRARY)
    set (APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    set (APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY})
    set (LIBS ${LIBS} ${APPLE_LIBS})
elseif (MOLLENOS)
  set (LIBS gallium-osmesa static_libddk)
endif ()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# this will be the directory the protocols are generated to
set (VIOARR_PROTOCOL_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protocols)

add_subdirectory (protocols)

add_subdirectory (libs/common)
add_subdirectory (libs/freetype)
add_subdirectory (libs/asgaard)
add_subdirectory (libs/glad)

# include subdirectories required for building apps
include_directories(libs/common/include/vioarr)
include_directories(libs/freetype/include)
include_directories(libs/asgaard/include)

add_subdirectory (apps/alumni)
add_subdirectory (apps/heimdall)

add_subdirectory (core)

# configure global install targets
install (EXPORT AsgaardExport DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/asgaard)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/protocols DESTINATION ${CMAKE_INSTALL_PREFIX}/share/vioarr/protocols)
