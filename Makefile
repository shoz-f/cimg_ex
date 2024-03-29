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
CFLAGS		+= -O2 -Isrc $(addprefix -I, $(EXTRA_LIB)) -pedantic -fPIC
LDFLAGS		+= -shared
ERL_CFLAGS	?= -I"$(ERL_EI_INCLUDE_DIR)"
ERL_LDFLAGS	?= -L"$(ERL_EI_LIBDIR)"
ifneq (,$(CROSSCOMPILE))
    NIFS = $(PRIV)/$(NIF_NAME).so
    LDFLAGS += -lm -lpthread
    CFLAGS  += -Dcimg_display=0
else ifneq (,$(findstring MSYS_NT,$(HOSTOS)))
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
	@echo "-CXX $(notdir $@)"
	$(CXX) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(NIFS): $(OBJS)
	@echo "-LD $(notdir $@)"
	$(CXX) $^ $(ERL_LDFLAGS) $(LDFLAGS) -o $@

$(PRIV) $(BUILD):
	mkdir -p $@

clean:
	$(RM) $(NIFS) $(BUILD)/*.o src/*.inc lib/cimg/$(NIF_NAME).ex

.PHONY: all clean setup build

################################################################################
# Download 3rd-party libraries
CIMG_VER    = 3.2.4
EXTRA_LIB   += ./3rd_party/CImg-$(CIMG_VER)
./3rd_party/CImg-$(CIMG_VER):
	@echo "-DOWNLOAD $(notdir $@)"
	mkdir -p 3rd_party
	cd ./3rd_party;\
	wget -q http://cimg.eu/files/CImg_$(CIMG_VER).zip;\
	unzip -q CImg_$(CIMG_VER).zip && rm CImg_*.zip

#STB_COMMIT  = 5736b15f7ea0ffb08dd38af21067c314d6a3aae9
EXTRA_LIB   += ./3rd_party/stb
./3rd_party/stb:
	@echo "-DOWNLOAD $(notdir $@)"
	mkdir -p 3rd_party
	cd ./3rd_party;\
	wget -q https://github.com/nothings/stb/archive/refs/heads/master.zip;\
	unzip -q master.zip && mv stb-master stb && rm master.zip

setup: $(EXTRA_LIB)

################################################################################
# NIF name
NIF_TABLE	+= src/cimg_nif.inc
src/cimg_nif.inc: src/cimg_nif.cc
	@echo "-GENERATE $(notdir $@)"
	python3 nif_tbl.py -o $@ --prefix cimg_ --ns NifCImgU8:: $<

NIF_TABLE	+= src/cimgdisplay_nif.inc
src/cimgdisplay_nif.inc: src/cimgdisplay_nif.h
	@echo "-GENERATE $(notdir $@)"
	python3 nif_tbl.py -o $@ --prefix cimgdisplay_ --ns NifCImgDisplay:: $<

NIF_STUB	= lib/cimg/$(NIF_NAME).ex
$(NIF_STUB): $(NIF_TABLE)
	@echo "-GENERATE $(notdir $@)"
	python3 nif_stub.py -o $@ CImg.NIF $^

CIMG_CMD_TABLE  += src/cimg_cmd.inc
src/cimg_cmd.inc: src/cimg_cmd.h
	@echo "-GENERATE $(notdir $@)"
	python3 cmd_tbl.py -o $@ --ns cmd_ $<

$(BUILD)/$(NIF_NAME).o: $(CIMG_CMD_TABLE) $(NIF_STUB)

# Don't echo commands unless the caller exports "V=1"
${V}.SILENT:
