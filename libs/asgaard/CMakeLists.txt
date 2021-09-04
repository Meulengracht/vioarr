
set (SRCS "")

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}/libs/asgaard" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
endmacro()

# setup include directories
include_directories(
    ${VIOARR_PROTOCOL_DIRECTORY}
    ${libgracht_SOURCE_DIR}/include
    ${zip_SOURCE_DIR}/src
    ${zip_SOURCE_DIR}/src/extlibs
    ${PROJECT_SOURCE_DIR}/libs/common/include
    ../freetype/include
    ./include/
)

set (GENERATED_SOURCES
    ${VIOARR_PROTOCOL_DIRECTORY}/hd_core_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_core_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_screen_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_memory_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_memory_pool_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_buffer_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_surface_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_pointer_service_client.c
    ${VIOARR_PROTOCOL_DIRECTORY}/wm_keyboard_service_client.c
)

if (MOLLENOS)
    set (GENERATED_SOURCES ${GENERATED_SOURCES} hid_events_service_client.c)
endif ()

set_source_files_properties ( ${GENERATED_SOURCES} PROPERTIES GENERATED TRUE )

# Configure primary executable target
add_sources (
    drawing/font_manager.cpp
    drawing/font.cpp
    drawing/image.cpp
    drawing/painter.cpp

    theming/theme_manager.cpp
    theming/theme.cpp

    widgets/icon.cpp
    widgets/label.cpp
    widgets/textbox.cpp

    application.cpp
    dispatcher.cpp
    environment.cpp
    key_event.cpp
    keyboard.cpp
    memory_buffer.cpp
    object_manager.cpp
    object.cpp
    pointer.cpp
    screen.cpp
    subsurface.cpp
    surface.cpp
    window_base.cpp
    window_decoration.cpp
    window_edge.cpp
    window_title.cpp
)

if (MOLLENOS)
    add_sources (application_vali.cpp memory_pool_vali.cpp)
elseif (WIN32)
    add_sources (application_win32.cpp)
elseif (UNIX)
    add_definitions(-Wno-unused-function)
    add_sources (application_unix.cpp memory_pool_unix.cpp)
endif ()

add_library(asgaard SHARED ${GENERATED_SOURCES} ${SRCS})
add_dependencies(asgaard zip wm_client_service hd_client_service)
target_compile_definitions(asgaard PRIVATE
    -DASGAARD_BUILD -DASGAARD_THEME_PASSWORD=\"$ENV{VALI_CODE}\"
)
target_link_libraries(asgaard freetype gracht zip bzip2 lzma z)

if (MOLLENOS)
  target_link_libraries(asgaard ValiDDK::libddk)
endif()

add_custom_command(TARGET asgaard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:asgaard>
)

# setup install targets
install(TARGETS asgaard freetype gracht zip bzip2 lzma png z zlib EXPORT AsgaardExport)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/asgaard DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_INSTALL_PREFIX}/share/asgaard)
