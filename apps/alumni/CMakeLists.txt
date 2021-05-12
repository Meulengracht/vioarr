
set (SRCS "")

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}/apps/alumni" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
endmacro()

add_sources (
    main.cpp
    terminal.cpp
    terminal_vt100.cpp
    terminal_line.cpp
    terminal_interpreter.cpp
    targets/resolver_base.cpp
)

if (MOLLENOS)
    add_sources (targets/resolver_vali.cpp)
elseif (WIN32)
    add_sources (targets/resolver_win32.cpp)
endif ()

# Configure primary executable target
add_executable(alumni ${SRCS})
if (MOLLENOS)
  target_link_libraries(alumni asgaard static_libddk static_libgracht)
endif()
