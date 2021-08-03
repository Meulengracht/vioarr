
include_directories (include)

add_library (common STATIC src/list.c)

# setup install targets
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/vioarr DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
