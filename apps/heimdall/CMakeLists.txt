include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (GENERATED_SOURCES
    #hd_core_protocol_server.c
)
set_source_files_properties ( ${GENERATED_SOURCES} PROPERTIES GENERATED TRUE )

include_directories (
    ${PROJECT_SOURCE_DIR}/libs/common/include
    ${PROJECT_SOURCE_DIR}/libs/asgaard/include
    ${libgracht_SOURCE_DIR}/include
)

# Configure primary executable target
add_executable(heimdall
    ${GENERATED_SOURCES}
    
    effects/guassian_blur.cpp
    main.cpp
)
#add_dependencies(heimdall protocols)
target_link_libraries(heimdall asgaard gracht)

if (MOLLENOS)
  target_link_libraries(heimdall static_libddk)
elseif (UNIX)
  target_link_libraries(heimdall -lpthread)
endif()

