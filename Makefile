# Makefile for building the NIF
#
# MIX_APP_PATH          path to the build directory
# ERL_EI_INCLUDE_DIR    include path to erlang header
# ERL_EI_LIBDIR         path to erlang/c libraries (Not necessaly for NIFs)
ifeq ($(MIX_APP_PATH),)
calling_from_make:
	mix compile
endif

PREFIX = $(MIX_APP_PATH)/priv
BUILD  = $(MIX_APP_PATH)/obj

ifeq ($(shell uname -s),Linux)
    NIF = $(PREFIX)/cimg_nif.so
else
    NIF = $(PREFIX)/cimg_nif.dll
endif

CFLAGS  ?= -O2 -Wall -Wextra -Wno-unused-parameter -pedantic -fPIC
LDFLAGS += -fPIC -shared -ljpeg #-Wl,--out-implib,a.lib

# Set Erlang-specific compile and linker flags
ERL_CFLAGS  ?= -I"$(ERL_EI_INCLUDE_DIR)"
ERL_LDFLAGS ?= -L"$(ERL_EI_LIBDIR)"

SRC = src/cimg_nif.cc
HEADERS =$(wildcard src/*.h)
OBJ = $(SRC:src/%.cc=$(BUILD)/%.o)

all: install

install: $(PREFIX) $(BUILD) $(NIF)

$(OBJ): $(HEADERS) Makefile

$(BUILD)/%.o: src/%.cc
	$(CXX) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(NIF): $(OBJ)
	$(CXX) $^ $(ERL_LDFLAGS) $(LDFLAGS) -o $@

$(PREFIX):
	mkdir -p $@

$(BUILD):
	mkdir -p $@

clean:
	$(RM) $(NIF) $(BUILD)/*.o

.PHONY: all clean install
