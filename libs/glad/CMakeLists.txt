
add_custom_command (
    OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/src/glad.c ${CMAKE_CURRENT_BINARY_DIR}/include/glad/glad.h ${CMAKE_CURRENT_BINARY_DIR}/include/KHR/khrplatform.h
    COMMAND python3 -m glad --generator=c --profile=core --out-path=${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_library (glad STATIC ${CMAKE_CURRENT_BINARY_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
