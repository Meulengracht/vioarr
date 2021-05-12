include_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../include)

set (GENERATED_HEADERS
    wm_buffer_service.h
    wm_core_service.h
    wm_keyboard_service.h
    wm_memory_pool_service.h
    wm_memory_service.h
    wm_pointer_service.h
    wm_screen_service.h
    wm_surface_service.h
)

set (GENERATED_SERVER_SOURCES
    wm_buffer_service_server.h
    wm_buffer_service_server.c
    wm_core_service_server.h
    wm_core_service_server.c
    wm_keyboard_service_server.h
    wm_keyboard_service_server.c
    wm_memory_pool_service_server.h
    wm_memory_pool_service_server.c
    wm_memory_service_server.h
    wm_memory_service_server.c
    wm_pointer_service_server.h
    wm_pointer_service_server.c
    wm_screen_service_server.h
    wm_screen_service_server.c
    wm_surface_service_server.h
    wm_surface_service_server.c
)

set (GENERATED_CLIENT_SOURCES
    wm_buffer_service_client.h
    wm_buffer_service_client.c
    wm_core_service_client.h
    wm_core_service_client.c
    wm_keyboard_service_client.h
    wm_keyboard_service_client.c
    wm_memory_pool_service_client.h
    wm_memory_pool_service_client.c
    wm_memory_service_client.h
    wm_memory_service_client.c
    wm_pointer_service_client.h
    wm_pointer_service_client.c
    wm_screen_service_client.h
    wm_screen_service_client.c
    wm_surface_service_client.h
    wm_surface_service_client.c
)

add_custom_command(
    OUTPUT  ${GENERATED_HEADERS} ${GENERATED_SERVER_SOURCES} ${GENERATED_CLIENT_SOURCES}
    COMMAND python3 ${GRACHT_INSTALL_DIR}/share/gracht/parser.py --service ${CMAKE_CURRENT_SOURCE_DIR}/vioarr.gr --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --server --client
    DEPENDS gracht ${CMAKE_CURRENT_SOURCE_DIR}/types.gr ${CMAKE_CURRENT_SOURCE_DIR}/vioarr.gr
)

add_custom_target(
    wm_server_service
    DEPENDS ${GENERATED_HEADERS} ${GENERATED_SERVER_SOURCES}
)

add_custom_target(
    wm_client_service
    DEPENDS ${GENERATED_HEADERS} ${GENERATED_CLIENT_SOURCES}
)
