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

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (GENERATED_SOURCES
    ${VIOARR_PROTOCOL_DIRECTORY}/hd_core_service_server.c
)
set_source_files_properties ( ${GENERATED_SOURCES} PROPERTIES GENERATED TRUE )

include_directories (
    ${VIOARR_PROTOCOL_DIRECTORY}
    ${PROJECT_SOURCE_DIR}/libs/common/include
    ${PROJECT_SOURCE_DIR}/libs/asgaard/include
    ${libgracht_SOURCE_DIR}/include
)

if (MOLLENOS)
    include_directories($ENV{VALI_DDK_PATH}/include)
endif()

add_sources(
    effects/guassian_blur.cpp
    utils/spawner.cpp
    main.cpp
)
if (MOLLENOS)
  add_sources(utils/register_vali.cpp)
elseif (UNIX)
  add_sources(utils/register_unix.cpp)
endif()

# Configure primary executable target
add_executable(heimdall ${GENERATED_SOURCES} ${SRCS})
add_dependencies(heimdall hd_server_service)
target_link_libraries(heimdall asgaard gracht)

if (MOLLENOS)
  target_link_libraries(heimdall static_libddk)
elseif (UNIX)
  target_link_libraries(heimdall -lpthread)
endif()

