include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories (
    ${PROJECT_SOURCE_DIR}/libs/common/include
    ${PROJECT_SOURCE_DIR}/libs/asgaard/include
)

# Configure primary executable target
add_executable(store
    main.cpp
)

target_link_libraries(store asgaard)

if (MOLLENOS)
  target_link_libraries(store static_libddk)
elseif (UNIX)
  target_link_libraries(store -lpthread)
endif()

