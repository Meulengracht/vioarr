
set (SRCS "")

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}/core" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
endmacro()

include_directories (
    ${CMAKE_BINARY_DIR}/protocols
    ${GRACHT_INSTALL_DIR}/include
    ${CMAKE_BINARY_DIR}/libs/glad/include/glad
)

if (WIN32)
    add_sources (
        engine/core/vioarr_engine_glfw.c
        engine/memory/vioarr_ram_win32.c
        engine/screen/vioarr_screen_glfw.c
    )
elseif (UNIX)
    add_definitions(-Wall -Wextra -Wno-unused-function)
    add_sources (
        engine/core/vioarr_engine_glfw.c
        engine/memory/vioarr_ram_unix.c
        engine/screen/vioarr_screen_glfw.c
    )
elseif (MOLLENOS)
    add_definitions(-DVIOARR_REVERSE_FB_BLIT -DGL_GLEXT_PROTOTYPES)
    add_definitions(-Wall -Wextra -Wno-unused-function)
    add_sources (
        engine/core/vioarr_engine_vali.c
        engine/memory/vioarr_ram_vali.c
        engine/screen/vioarr_screen_osmesa.c
        vioarr_hid.c
    )
endif ()

# setup some configuration parameters for the build
add_definitions(-DVIOARR_BACKEND_NANOVG)

# add the protocol sources that handles communication
add_sources (
    vioarr.c
    vioarr_surface.c
    vioarr_screen.c
    vioarr_pointer.c
    vioarr_memory.c
    vioarr_core.c
)

# add the engine sources
add_sources (
    engine/backend/nanovg/nanovg.c
    engine/vioarr_buffer.c
    engine/vioarr_input.c
    engine/vioarr_manager.c
    engine/vioarr_objects.c
    engine/vioarr_region.c
    engine/vioarr_renderer.c
    engine/vioarr_surface.c
    engine/vioarr_utils.c
)

add_executable(vioarr ${SRCS})
target_link_libraries(vioarr ${LIBS})
