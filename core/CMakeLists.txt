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
    ${VIOARR_PROTOCOL_DIRECTORY}
    ${libgracht_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/../libs/glad/include/glad
    ${freetype_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/libs/common/include
)

if (MOLLENOS)
    include_directories($ENV{VALI_DDK_PATH}/include)
endif()

set (GENERATED_SOURCES
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_core_service_server.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_screen_service_server.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_memory_service_server.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_memory_pool_service_server.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_buffer_service_server.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_surface_service_server.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_pointer_service_server.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_keyboard_service_server.c
)

if (MOLLENOS)
    set (GENERATED_SOURCES ${GENERATED_SOURCES} ${VIOARR_PROTOCOL_DIRECTORY}/ctt_input_service_client.c)
endif ()

set_source_files_properties ( ${GENERATED_SOURCES} PROPERTIES GENERATED TRUE )


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

    if ("$ENV{VALI_ARCH}" STREQUAL "amd64")
        set (CMAKE_ASM_NASM_OBJECT_FORMAT win64)
        add_sources(engine/screen/vioarr_screen_amd64.s)
        set_source_files_properties(engine/screen/vioarr_screen_amd64.s PROPERTIES LANGUAGE ASM_NASM)
    elseif ("$ENV{VALI_ARCH}" STREQUAL "i386")
        set (CMAKE_ASM_NASM_OBJECT_FORMAT win32)
        add_sources(engine/screen/vioarr_screen_i386.s)
        set_source_files_properties(engine/screen/vioarr_screen_i386.s PROPERTIES LANGUAGE ASM_NASM)
    endif()
    set (CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> \
      <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
    set (CMAKE_ASM_NASM_FLAGS "-Xvc -D$ENV{VALI_ARCH} -D__$ENV{VALI_ARCH}__")

    add_sources (
        engine/core/vioarr_engine_vali.c
        engine/memory/vioarr_ram_vali.c
        engine/screen/vioarr_screen_osmesa.c
        vioarr_hid.c
    )
endif ()

# setup some configuration parameters for the build
add_definitions(-DVIOARR_BACKEND_NANOVG -DNANOVG_GL3_IMPLEMENTATION -DFONS_USE_FREETYPE)

# add the protocol sources that handles communication
add_sources (
    ${GENERATED_SOURCES}

    vioarr.c
    vioarr_surface.c
    vioarr_screen.c
    vioarr_pointer.c
    vioarr_keyboard.c
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
add_dependencies(vioarr glad freetype wm_server_service)
if (MOLLENOS)
    add_dependencies(vioarr os_client_service)
endif ()
target_link_libraries(vioarr common gracht glad ${LIBS})

# Setup install targets for vioarr
install(TARGETS vioarr
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
