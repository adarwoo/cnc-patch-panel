TOP:=.
ARCH:=attiny3227
BIN:=patch_panel
DATAGRAM:=conf/datagram.hpp

# Headers in src and conf directories
INCLUDE_DIRS:=conf src

# ASX modules to use
ASX_USE:=pca9555 modbus_rtu

# Project own files
SRCS = \
   src/main.cpp \
   src/iomux.cpp \
   src/patch.cpp \
   src/modbus.cpp \
   src/door.cpp \

# Inlude the actual build rules
include asx/make/rules.mak

# Add dependency to generate the datagram from the config
src/modbus.cpp : $(DATAGRAM)

# Turn off ULOG logging
CPPFLAGS += -DULOG_LEVEL=-1

# Clean the datagram file on clean
CLEAN_FILES += $(DATAGRAM)
