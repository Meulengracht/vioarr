include_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../include)

set (WM_GENERATED_HEADERS
    wm_buffer_service.h
    wm_core_service.h
    wm_keyboard_service.h
    wm_memory_pool_service.h
    wm_memory_service.h
    wm_pointer_service.h
    wm_screen_service.h
    wm_surface_service.h
)

set (WM_GENERATED_SERVER_SOURCES
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

set (WM_GENERATED_CLIENT_SOURCES
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
    OUTPUT  ${WM_GENERATED_HEADERS} ${WM_GENERATED_SERVER_SOURCES} ${WM_GENERATED_CLIENT_SOURCES}
    COMMAND python3 $ENV{VALI_SDK_PATH}/share/vali/parser.py --service ${CMAKE_CURRENT_SOURCE_DIR}/vioarr.gr --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --server --client
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/types.gr ${CMAKE_CURRENT_SOURCE_DIR}/vioarr.gr
)

add_custom_target(
    wm_server_service
    DEPENDS ${WM_GENERATED_HEADERS} ${WM_GENERATED_SERVER_SOURCES}
)

add_custom_target(
    wm_client_service
    DEPENDS ${WM_GENERATED_HEADERS} ${WM_GENERATED_CLIENT_SOURCES}
)

set (HD_GENERATED_HEADERS
    hd_core_service.h
)

set (HD_GENERATED_SERVER_SOURCES
    hd_core_service_server.h
    hd_core_service_server.c
)

set (HD_GENERATED_CLIENT_SOURCES
    hd_core_service_client.h
    hd_core_service_client.c
)

add_custom_command(
    OUTPUT  ${HD_GENERATED_HEADERS} ${HD_GENERATED_SERVER_SOURCES} ${HD_GENERATED_CLIENT_SOURCES}
    COMMAND python3 $ENV{VALI_SDK_PATH}/share/vali/parser.py --service ${CMAKE_CURRENT_SOURCE_DIR}/heimdall.gr --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --server --client
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/heimdall.gr
)

add_custom_target(
    hd_server_service
    DEPENDS ${HD_GENERATED_HEADERS} ${HD_GENERATED_SERVER_SOURCES}
)

add_custom_target(
    hd_client_service
    DEPENDS ${HD_GENERATED_HEADERS} ${HD_GENERATED_CLIENT_SOURCES}
)

if (MOLLENOS)
    set (OS_GENERATED_HEADERS
        ctt_input_service.h
        sys_device_service.h
    )

    set (OS_GENERATED_CLIENT_SOURCES
        ctt_input_service_client.h
        ctt_input_service_client.c
        sys_device_service_client.h
        sys_device_service_client.c
    )

    add_custom_command(
        OUTPUT  ${OS_GENERATED_HEADERS} ${OS_GENERATED_CLIENT_SOURCES}
        COMMAND python3 $ENV{VALI_SDK_PATH}/share/vali/parser.py --service $ENV{VALI_SDK_PATH}/share/vali/protocols/contracts/input.gr --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --client
        COMMAND python3 $ENV{VALI_SDK_PATH}/share/vali/parser.py --service $ENV{VALI_SDK_PATH}/share/vali/protocols/device.gr --out ${CMAKE_CURRENT_BINARY_DIR} --lang-c --client
        DEPENDS $ENV{VALI_SDK_PATH}/share/vali/protocols/device.gr $ENV{VALI_SDK_PATH}/share/vali/protocols/contracts/input.gr
    )

    add_custom_target(
        os_client_service
        DEPENDS ${OS_GENERATED_HEADERS} ${OS_GENERATED_CLIENT_SOURCES}
    )
endif()
