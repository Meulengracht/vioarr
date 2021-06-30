include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (GENERATED_SOURCES
    ${CMAKE_BINARY_DIR}/protocols/hd_core_service_server.c
)
set_source_files_properties ( ${GENERATED_SOURCES} PROPERTIES GENERATED TRUE )

include_directories (
    ${CMAKE_BINARY_DIR}/protocols
    ${PROJECT_SOURCE_DIR}/libs/common/include
    ${PROJECT_SOURCE_DIR}/libs/asgaard/include
    ${libgracht_SOURCE_DIR}/include
)

# Configure primary executable target
add_executable(heimdall
    ${GENERATED_SOURCES}
    
    effects/guassian_blur.cpp
    utils/register.cpp
    utils/spawner.cpp
    main.cpp
)
add_dependencies(heimdall hd_server_service)
target_link_libraries(heimdall asgaard gracht)

if (MOLLENOS)
  target_link_libraries(heimdall static_libddk)
elseif (UNIX)
  target_link_libraries(heimdall -lpthread)
endif()

