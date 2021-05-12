include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
