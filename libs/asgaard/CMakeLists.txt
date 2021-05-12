# setup include directories
include_directories(${GRACHT_INSTALL_DIR}/include)

set (GENERATED_SOURCES
    ${CMAKE_BINARY_DIR}/protocols/wm_core_service_client.c
    ${CMAKE_BINARY_DIR}/protocols/wm_screen_service_client.c
    ${CMAKE_BINARY_DIR}/protocols/wm_memory_service_client.c
    ${CMAKE_BINARY_DIR}/protocols/wm_memory_pool_service_client.c
    ${CMAKE_BINARY_DIR}/protocols/wm_buffer_service_client.c
    ${CMAKE_BINARY_DIR}/protocols/wm_surface_service_client.c
    ${CMAKE_BINARY_DIR}/protocols/wm_pointer_service_client.c
    ${CMAKE_BINARY_DIR}/protocols/wm_keyboard_service_client.c
)

if (MOLLENOS)
    set (GENERATED_SOURCES ${GENERATED_SOURCES} hid_events_service_client.c)
endif ()

set_source_files_properties ( ${GENERATED_SOURCES} PROPERTIES GENERATED TRUE )

# Configure primary executable target
add_library(asgaard SHARED
    ${GENERATED_SOURCES}
    
    drawing/font_manager.cpp
    drawing/font.cpp
    drawing/image.cpp
    drawing/painter.cpp

    theming/theme_manager.cpp
    theming/theme.cpp

    widgets/icon.cpp
    widgets/label.cpp

    application.cpp
    key_event.cpp
    memory_buffer.cpp
    memory_pool.cpp
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

add_dependencies(asgaard wm_client_service)
target_compile_definitions(asgaard PRIVATE
    -DASGAARD_BUILD -DASGAARD_THEME_PASSWORD=\"$ENV{VALI_CODE}\"
)

if (MOLLENOS)
  target_link_libraries(asgaard static_zip static_z static_lzma static_bzip2 static_libgracht libfreetype static_libddk)
endif()
