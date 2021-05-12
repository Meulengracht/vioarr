# Makefile for building a generic userspace application
# Include all the definitions for os

INCLUDES =-I$(VALI_DDK_PATH)/include -I$(VALI_DDK_PATH)/include/wm -Iloader/include
LFLAGS := -LIBPATH:$(VALI_DDK_PATH)/lib static_libds.lib static_libddk.lib static_libgracht.lib libfreetype.lib

ifeq ($(VALI_ARCH),i386)
	ASFLAGS = -f win32 -Xvc -DVIOARR_REVERSE_FB_BLIT
else
	ASFLAGS = -f win64 -Xvc -DVIOARR_REVERSE_FB_BLIT
endif

# The different types of renderes available
APP_OSMESA_DEFINES := -D__VIOARR_CONFIG_RENDERER_OSMESA -DVIOARR_REVERSE_FB_BLIT -DGL_GLEXT_PROTOTYPES
APP_OSMESA_SOURCES_C := loader/src/glad.c engine/screen/vioarr_screen_osmesa.c
APP_OSMESA_LFLAGS := gallium-osmesa.lib

APP_DIRECT_DEFINES := 
APP_DIRECT_SOURCES_C :=
APP_DIRECT_LFLAGS :=

APP_HEADLESS_DEFINES := -D__VIOARR_CONFIG_RENDERER_HEADLESS
APP_HEADLESS_SOURCES_C := engine/screen/vioarr_screen_headless.c
APP_HEADLESS_LFLAGS :=

# The different types of backends available
APP_NANOVG_DEFINES := -DVIOARR_BACKEND_NANOVG -DNANOVG_GL3_IMPLEMENTATION  -DFONS_USE_FREETYPE
APP_NANOVG_SOURCES_C := $(wildcard engine/backend/nanovg/*.c)

APP_BLEND2D_DEFINES := -DVIOARR_BACKEND_BLEND2D

APP_SOURCES_ASM += engine/screen/vioarr_screen_$(VALI_ARCH).s
APP_SOURCES_C := $(wildcard engine/memory/*.c) \
				 $(wildcard engine/*.c) \
		         $(wildcard *.c)
		         
APP_PROTOCOLS_C := protocols/wm_core_protocol_server.c \
				   protocols/wm_screen_protocol_server.c \
				   protocols/wm_memory_protocol_server.c \
				   protocols/wm_memory_pool_protocol_server.c \
				   protocols/wm_buffer_protocol_server.c \
				   protocols/wm_surface_protocol_server.c \
				   protocols/wm_pointer_protocol_server.c \
				   protocols/wm_keyboard_protocol_server.c \
				   protocols/ctt_input_protocol_client.c \
				   protocols/svc_device_protocol_client.c

APP_DEFINES := 
ifdef VALI_HEADLESS
APP_SOURCES_C += $(APP_HEADLESS_SOURCES_C)
APP_DEFINES += $(APP_HEADLESS_DEFINES)
LFLAGS += $(APP_HEADLESS_LFLAGS)
else
APP_SOURCES_C := $(APP_OSMESA_SOURCES_C) $(APP_NANOVG_SOURCES_C) $(APP_SOURCES_C)
APP_DEFINES += $(APP_OSMESA_DEFINES) $(APP_NANOVG_DEFINES)
LFLAGS += $(APP_OSMESA_LFLAGS)
endif

APP_CFLAGS = $(VALI_CFLAGS) $(CFLAGS) $(VALI_INCLUDES) $(INCLUDES) $(APP_DEFINES)
APP_CXXFLAGS = $(VALI_CXXFLAGS) $(CXXFLAGS) $(VALI_INCLUDES) $(INCLUDES) $(APP_DEFINES)
APP_LFLAGS = /entry:__CrtConsoleEntry $(VALI_LFLAGS) $(VALI_SDK_CXXLIBS) $(LFLAGS)

APP_OBJECTS = $(APP_PROTOCOLS_C:.c=.o) $(APP_SOURCES_C:.c=.o) $(APP_SOURCES_ASM:.s=.o)

.PHONE: all
all: protocols loader $(VALI_APPLICATION_PATH)/bin/vioarr.app

$(VALI_APPLICATION_PATH)/bin/vioarr.app: $(APP_OBJECTS)
	@printf "%b" "\033[0;36mCreating application " $@ "\033[m\n"
	@$(LD) $(APP_LFLAGS) $(APP_OBJECTS) /out:$@

protocols:
	@mkdir -p $@

protocols/wm_core_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/wm_screen_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/wm_memory_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/wm_memory_pool_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/wm_buffer_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/wm_surface_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/wm_pointer_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/wm_keyboard_protocol_server.c: ../protocols/wm_protocol.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --lang-c --server --out ./protocols/

protocols/ctt_input_protocol_client.c: $(VALI_DDK_PATH)/share/vali/protocols/contract_protocols.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --include input --lang-c --client --out ./protocols/

protocols/svc_device_protocol_client.c: $(VALI_DDK_PATH)/share/vali/protocols/service_protocols.xml
	@printf "%b" "\033[0;36mRegenerating protocol " $@ "\033[m\n"
	python3 $(VALI_DDK_PATH)/share/vali/parser.py --protocol $< --include device --lang-c --client --out ./protocols/

loader:
	@mkdir -p $@

loader/src/glad.c: loader
	@printf "%b" "\033[0;36mRegenerating opengl loader files " $@ "\033[m\n"
	@pip3 install --upgrade git+https://github.com/meulengracht/glad.git#egg=glad
	@python3 -m glad --generator=c --profile=core --out-path=loader

%.o : %.cpp
	@printf "%b" "\033[0;32mCompiling C++ source object " $< "\033[m\n"
	@$(CC) -c $(APP_CXXFLAGS) -o $@ $<

%.o : %.c
	@printf "%b" "\033[0;32mCompiling C source object " $< "\033[m\n"
	@$(CC) -c $(APP_CFLAGS) -o $@ $<

%.o : %.s
	@printf "%b" "\033[0;32mAssembling source object " $< "\033[m\n"
	@$(AS) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -rf loader/*
	@rm -f protocols/*
	@rm -f $(APP_OBJECTS)
