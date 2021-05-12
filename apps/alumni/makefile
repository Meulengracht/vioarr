# Makefile for building a generic userspace application
# Include all the definitions for os
APP_NAME := alumni
APP_SOURCES_CXX := targets/resolver_base.cpp \
				   targets/resolver_vali.cpp \
			       $(wildcard *.cpp)

CXXFLAGS := -O3 -std=c++17
INCLUDES := -I$(VALI_DDK_PATH)/include
LFLAGS := -LIBPATH:$(VALI_DDK_PATH)/lib asgaard.lib static_libddk.lib

include ../application.mk