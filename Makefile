TOP:=.
ARCH:=attiny3227
BIN:=patch_panel
INCLUDE_DIRS:=conf src

ASX_USE:=pca9555 modbus_rtu trace

# Project own files
SRCS = \
   src/main.cpp \
   src/iomux.cpp \
   src/patch.cpp \
   src/modbus.cpp \

# Inlude the actual build rules
include asx/make/rules.mak

DATAGRAM = conf/datagram.hpp

# Add dependency to generate the datagram from the config
src/main.cpp : $(DATAGRAM)
