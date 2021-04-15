# Makefile for minGW-W64

# default conmpiler
CXX = $(PREFIX)g++
CC  = $(PREFIX)gcc
LNK = $(PREFIX)g++
WRC = $(PREFIX)windres

# where is libusbk ?
LIBUSBK_ARC = amd64
LIBUSBK_DIR = ../libusbK-dev-kit
LIBUSBK_INC = $(LIBUSBK_DIR)/includes
LIBUSBK_DLL = $(LIBUSBK_DIR)/bin/dll/$(LIBUSBK_ARC)/libusbK.dll

# default options
DIR_SRC = src
DIR_BIN = bin
DIR_OBJ = obj
EXT_BIN = .exe
TGT_BIN = rkdeveloptool
TARGET  = $(DIR_BIN)/$(TGT_BIN)$(EXT_BIN)

# compiler options
CFLAGS += -I$(DIR_SRC)
CFLAGS += -I$(LIBUSBK_INC)
CFLAGS += -Ires
CFLAGS += -I/mingw64/include/libusb-1.0
LFLAGS += $(LIBUSBK_DLL)
LFLAGS += -lusb-1.0
LFLAGS += -s -O2 -static

# Windows Resource Flags
WFLAGS  = $(CFLAGS)

# Sources
SRCS = $(wildcard $(DIR_SRC)/*.cpp)

# Windows resource
WRES = res/resource.rc

# Make object targets from SRCS.
OBJS = $(SRCS:$(DIR_SRC)/%.cpp=$(DIR_OBJ)/%.o)
WROBJ = $(DIR_OBJ)/resource.o

.PHONY: prepare clean

all: prepare continue

continue: $(TARGET)

prepare:
	@mkdir -p $(DIR_OBJ)
	@mkdir -p $(DIR_BIN)

clean:
	@echo "Cleaning built targets ..."
	@rm -rf $(OBJS)
	@rm -rf $(TARGET)

$(OBJS): $(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	@echo "Building $@ ... "
	@$(CXX) $(CFLAGS) -c $< -o $@

$(WROBJ): $(WRES) res/resource.h
	@echo "Building windows resource ..."
	@$(WRC) $(WFLAGS) -i $< -o $@

$(TARGET): $(OBJS) $(WROBJ)
	@echo "Generating $@ ..."
	@$(LNK) $^ $(LFLAGS) -o $@
	@echo "done."

