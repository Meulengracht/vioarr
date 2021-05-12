include_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../include)

add_custom_command(
    OUTPUT  test_utils_service_server.c test_utils_service_server.h test_utils_service_client.c test_utils_service_client.h test_utils_service.h
    COMMAND python3 ${CMAKE_SOURCE_DIR}/generator/parser.py --service ${CMAKE_CURRENT_SOURCE_DIR}/protocols/test_service.gr --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --server --client
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocols/test_service.gr
)
add_custom_target(
    test_protocols
    DEPENDS test_utils_service_server.c test_utils_service_client.c
)
