
add_custom_command (
    OUTPUT  glad.c
    COMMAND pip3 install --upgrade git+https://github.com/meulengracht/glad.git#egg=glad
    COMMAND python3 -m glad --generator=c --profile=core --out-path=loader
)

add_library (glad glad.c)
