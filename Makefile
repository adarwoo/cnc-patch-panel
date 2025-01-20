TOP:=.
ARCH:=attiny3227
BIN:=cnc_patch_panel
INCLUDE_DIRS:=conf src

ASX_USE:=pca9555 modbus_rtu

# Project own files
SRCS = \
   src/main.cpp \
   src/patch.cpp \

# Inlude the actual build rules
include asx/make/rules.mak

# Add dependency to generate the datagram from the config
src/main.cpp : conf/datagram.hpp
