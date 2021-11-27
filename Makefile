# Makefile for building the NIF
#
# MIX_APP_PATH          path to the build directory
# ERL_EI_INCLUDE_DIR    include path to erlang header
# ERL_EI_LIBDIR         path to erlang/c libraries (Not necessaly for NIFs)
ifeq ($(MIX_APP_PATH),)
calling_from_make:
	mix compile
endif

HOSTOS = $(shell uname -s)

PREFIX = $(MIX_APP_PATH)/priv
BUILD  = $(MIX_APP_PATH)/obj

DOWNLOAD = ./3d_party
LIB_CIMG = $(DOWNLOAD)/CImg
LIB_STB  = $(DOWNLOAD)/stb

CFLAGS  ?= -O2 -Isrc -I$(LIB_CIMG) -I$(LIB_STB) -pedantic -fPIC
LDFLAGS ?= -shared
ifneq (,$(findstring MSYS_NT,$(HOSTOS)))
    NIF = $(PREFIX)/cimg_nif.dll
    ifneq (,$(findstring $(MIX_ENV), dev test))
        LDFLAGS += -lgdi32
    else
        CFLAGS  += -Dcimg_display=0
    endif
else ifeq (Linux, $(HOSTOS))
    NIF = $(PREFIX)/cimg_nif.so
    LDFLAGS += -lm -lpthread
    ifneq (,$(findstring $(MIX_ENV), dev test))
        LDFLAGS += -L/usr/X11R6/lib -lX11
    else
        CFLAGS  += -Dcimg_display=0
    endif
else
    $(error Not available system "$(HOSTOS)")
endif

# Set Erlang-specific compile and linker flags
ERL_CFLAGS  ?= -I"$(ERL_EI_INCLUDE_DIR)"
ERL_LDFLAGS ?= -L"$(ERL_EI_LIBDIR)"

SRC = src/cimg_nif.cc
HEADERS =$(wildcard src/*.h)
OBJ = $(SRC:src/%.cc=$(BUILD)/%.o)

all: install

install: $(PREFIX) $(BUILD) $(DOWNLOAD) $(LIB_CIMG) $(LIB_STB) $(NIF)

$(OBJ): $(HEADERS) Makefile

$(BUILD)/%.o: src/%.cc
	$(CXX) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(NIF): $(OBJ)
	$(CXX) $^ $(ERL_LDFLAGS) $(LDFLAGS) -o $@

$(PREFIX):
	mkdir -p $@

$(BUILD):
	mkdir -p $@

$(DOWNLOAD):
	mkdir -p $@

$(LIB_CIMG):
	wget http://cimg.eu/files/CImg_latest.zip
	unzip CImg_latest.zip -d tmp
	mv tmp/CImg* $(LIB_CIMG)
	rm CImg_latest.zip
	rmdir tmp

$(LIB_STB):
	git clone https://github.com/nothings/stb.git $(LIB_STB)

clean:
	$(RM) $(NIF) $(BUILD)/*.o

.PHONY: all clean install deps
