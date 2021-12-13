# Makefile for building the NIF
#
# MIX_APP_PATH          path to the build directory
# ERL_EI_INCLUDE_DIR    include path to erlang header
# ERL_EI_LIBDIR         path to erlang/c libraries (Not necessaly for NIFs)

ifeq ($(MIX_APP_PATH),)
calling_from_make:
	mix compile
endif

HOSTOS		= $(shell uname -s)
NIF_NAME	= cimg_nif

# Directories
PRIV		= $(MIX_APP_PATH)/priv
BUILD		= $(MIX_APP_PATH)/obj

# Build options
CFLAGS		?= -O2 -Isrc $(addprefix -I, $(EXTRA_LIB)) -pedantic -fPIC
LDFLAGS		?= -shared
ERL_CFLAGS	?= -I"$(ERL_EI_INCLUDE_DIR)"
ERL_LDFLAGS	?= -L"$(ERL_EI_LIBDIR)"
ifneq (,$(findstring MSYS_NT,$(HOSTOS)))
    NIFS = $(PRIV)/$(NIF_NAME).dll
    ifneq (,$(findstring $(MIX_ENV), dev test))
        LDFLAGS += -lgdi32
    else
        CFLAGS  += -Dcimg_display=0
    endif
else ifeq (Linux, $(HOSTOS))
    NIFS = $(PRIV)/$(NIF_NAME).so
    LDFLAGS += -lm -lpthread
    ifneq (,$(findstring $(MIX_ENV), dev test))
        LDFLAGS += -L/usr/X11R6/lib -lX11
    else
        CFLAGS  += -Dcimg_display=0
    endif
else
    $(error Not available system "$(HOSTOS)")
endif

# Target list
HDRS = $(wildcard src/*.h)
SRCS = $(wildcard src/*.cc)
OBJS = $(SRCS:src/%.cc=$(BUILD)/%.o)

# Build rules
all: setup build

setup: $(PRIV) $(BUILD)

build: $(NIFS)

$(BUILD)/%.o: src/%.cc $(HDRS) Makefile
	$(CXX) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(NIFS): $(OBJS)
	$(CXX) $^ $(ERL_LDFLAGS) $(LDFLAGS) -o $@

$(PRIV) $(BUILD):
	mkdir -p $@

clean:
	$(RM) $(NIFS) $(BUILD)/*.o src/*.inc

.PHONY: all clean setup build

define ex_path
endef

################################################################################
# Download 3rd-party libraries
EXTRA_LIB	+= ./3rd_party/CImg
./3rd_party/CImg:
	mkdir -p $(dir $@)
	wget http://cimg.eu/files/CImg_latest.zip
	unzip CImg_latest.zip -d tmp
	mv tmp/CImg* $@
	rm CImg_latest.zip
	rmdir tmp

EXTRA_LIB	+= ./3rd_party/stb
./3rd_party/stb:
	mkdir -p $(dir $@)
	git clone https://github.com/nothings/stb.git $@

setup: $(EXTRA_LIB)

################################################################################
# NIF name
NIF_TABLE	+= src/cimg_nif.inc
src/cimg_nif.inc: src/cimg_nif.h
	py nif_tbl.py -o $@ --prefix cimg_ --ns NifCImgU8:: $<

NIF_TABLE	+= src/cimgdisplay_nif.inc
src/cimgdisplay_nif.inc: src/cimgdisplay_nif.h
	py nif_tbl.py -o $@ --prefix cimgdisplay_ --ns NifCImgDisplay:: $<

NIF_STUB	= lib/cimg/$(NIF_NAME).ex
$(NIF_STUB): $(NIF_TABLE)
	py nif_stub.py -o $@ CImg.NIF $^ src/cimgdisplay_nif.ext

$(BUILD)/$(NIF_NAME).o: $(NIF_STUB)
