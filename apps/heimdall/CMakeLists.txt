cmake_minimum_required(VERSION 3.8)
project(heimdall C CXX)

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
    hd_core_protocol_server.c
)

add_custom_command(
    OUTPUT  ${GENERATED_SOURCES}
    COMMAND python $ENV{VALI_DDK_PATH}/share/vali/parser.py --protocol ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/hd_protocol.xml --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --server
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/hd_protocol.xml
)
add_custom_target(
    protocols
    DEPENDS ${GENERATED_SOURCES}
)

# Configure primary executable target
add_executable(heimdall
    ${GENERATED_SOURCES}
    
    effects/guassian_blur.cpp
    main.cpp
)
add_dependencies(heimdall protocols)

if (MOLLENOS)
  target_link_libraries(heimdall asgaard static_libddk static_libgracht)
endif()

# Add a new install target
install(TARGETS heimdall
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# install the generated map file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/heimdall.map DESTINATION bin)
