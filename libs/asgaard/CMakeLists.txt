cmake_minimum_required(VERSION 3.8)
project(asgaard C CXX)

# setup include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
if (MOLLENOS)
  include_directories($ENV{VALI_APPLICATION_PATH}/include)
  include_directories($ENV{VALI_DDK_PATH}/include)

  link_directories($ENV{VALI_APPLICATION_PATH}/lib)
  link_directories($ENV{VALI_DDK_PATH}/lib)
endif()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set (GENERATED_SOURCES
    wm_core_protocol_client.c
    wm_screen_protocol_client.c
    wm_memory_protocol_client.c
    wm_memory_pool_protocol_client.c
    wm_buffer_protocol_client.c
    wm_surface_protocol_client.c
    wm_pointer_protocol_client.c
    wm_keyboard_protocol_client.c 
    hid_events_protocol_client.c
)

add_custom_command(
    OUTPUT  ${GENERATED_SOURCES}
    COMMAND python $ENV{VALI_DDK_PATH}/share/vali/parser.py --protocol ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/wm_protocol.xml --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --client
    COMMAND python $ENV{VALI_DDK_PATH}/share/vali/parser.py --protocol $ENV{VALI_DDK_PATH}/share/vali/protocols/hid_protocol.xml --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --client
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/wm_protocol.xml $ENV{VALI_DDK_PATH}/share/vali/protocols/hid_protocol.xml
)
add_custom_target(
    protocols
    DEPENDS ${GENERATED_SOURCES}
)

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
add_dependencies(asgaard protocols)
target_compile_definitions(asgaard PRIVATE
    -DASGAARD_BUILD -DASGAARD_THEME_PASSWORD=\"$ENV{VALI_CODE}\"
)

if (MOLLENOS)
  target_link_libraries(asgaard static_zip static_z static_lzma static_bzip2 static_libgracht libfreetype static_libddk)
endif()

# Install library headers
install(DIRECTORY include/ DESTINATION include/asgaard)

# Add a new install target
install(TARGETS asgaard
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# install the generated map file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/asgaard.map DESTINATION bin)
