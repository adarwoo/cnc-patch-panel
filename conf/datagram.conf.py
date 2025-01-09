#!/usr/bin/env python3
from modbus_rtu_slave_rc import *  # Import everything from modbus_generator

# Coils = LED+ (write multiple only)
# Discrete inputs = Switch state
# Input register = Active key

# Coil
# --------------
# 0 push_door
# 1 pull_door
# 2 blast_toolsetter
# 3 unclamp_chuck
# 4 blast_spindle

# discrete input
# --------------
# 0 Pressure readout


Modbus({
    "buffer_size": 16,
    "namespace": "patch",

    "callbacks": {
        "on_console_reply"   : [(u8, "switches"), (u8, "push_buttons")],
        "on_relay_ok"        : [],
        "on_pneumatic_reply" : [(u8, "pressure")],
    },

    # Console
    "device@37": [
        (CUSTOM,                u8(alias="switches"), u8(alias="push_buttons"), "on_console_reply"),
    ],

    # Relay - only use the write multiple
    "device@44": [
        (WRITE_MULTIPLE_COILS,  u16(0), # The product write all relays from 0
                                u16(3), # 3 relays read
                                u8(1),
                                u16(alias="data"),
                                "on_relay_ok"),
    ],

    # Pneumatic
    "device@49": [
        (CUSTOM,                u8(alias="pressure_input"), "on_pneumatic_reply"),
    ]
})
