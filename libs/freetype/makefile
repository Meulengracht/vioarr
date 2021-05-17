# Makefile for building a generic userspace application or library
SOURCES = src/autofit/autofit.c \
		  src/base/ftbase.c \
		  src/base/ftbbox.c \
		  src/base/ftbdf.c \
		  src/base/ftbitmap.c \
		  src/base/ftcid.c \
		  src/base/ftfntfmt.c \
		  src/base/ftfstype.c \
		  src/base/ftgasp.c \
		  src/base/ftglyph.c \
		  src/base/ftgxval.c \
		  src/base/ftinit.c \
		  src/base/ftlcdfil.c \
		  src/base/ftmm.c \
		  src/base/ftotval.c \
		  src/base/ftpatent.c \
		  src/base/ftpfr.c \
		  src/base/ftstroke.c \
		  src/base/ftsynth.c \
		  src/base/ftsystem.c \
		  src/base/fttype1.c \
		  src/base/ftwinfnt.c \
		  src/base/ftdebug.c \
		  src/bdf/bdf.c \
		  src/bzip2/ftbzip2.c \
		  src/cache/ftcache.c \
		  src/cff/cff.c \
		  src/cid/type1cid.c \
		  src/gzip/ftgzip.c \
		  src/lzw/ftlzw.c \
		  src/pcf/pcf.c \
		  src/pfr/pfr.c \
		  src/psaux/psaux.c \
		  src/pshinter/pshinter.c \
		  src/psnames/psnames.c \
		  src/raster/raster.c \
		  src/sfnt/sfnt.c \
		  src/smooth/smooth.c \
		  src/truetype/truetype.c \
		  src/type1/type1.c \
		  src/type42/type42.c \
		  src/winfonts/winfnt.c \
		  main.c
OBJECTS = $(SOURCES:.c=.o)

CONFIG_FLAGS = -DFT2_BUILD_LIBRARY -DFT_SHARED_LIBRARY
CFLAGS = $(VALI_CFLAGS) -O3 $(CONFIG_FLAGS) $(VALI_INCLUDES) -Iinclude
LFLAGS = /version:2.9 /entry:__CrtLibraryEntry /dll $(VALI_LFLAGS) $(VALI_SDK_CLIBS) z.lib libpng.lib

.PHONY: all
all: $(VALI_APPLICATION_PATH)/bin/libfreetype.dll

$(VALI_APPLICATION_PATH)/bin/libfreetype.dll: $(OBJECTS)
	@printf "%b" "\033[0;36mCreating shared library " $@ "\033[m\n"
	@$(LD) $(LFLAGS) $(OBJECTS) /out:$@
	@mv -f $(VALI_APPLICATION_PATH)/bin/libfreetype.lib $(VALI_APPLICATION_PATH)/lib/libfreetype.lib
	@cp -a include/* $(VALI_APPLICATION_PATH)/include/
	@rm -rf $(VALI_APPLICATION_PATH)/include/freetype/internal
	
%.o : %.c
	@printf "%b" "\033[0;32mCompiling C source object " $< "\033[m\n"
	@$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	@rm -f $(OBJECTS)