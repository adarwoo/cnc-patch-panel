#!/usr/bin/env python3
from modbus_rtu_slave_rc import *  # Import everything from modbus_generator

Modbus({
    "mode": "master",        # Tell the generator we're a master
    "buffer_size": 16,       # Set the buffer size to at least 16 bytes
    "namespace": "patch",    # The datagram will be patch::Datagram

    # Prototypes for the reply callbacks
    "callbacks": {
        "on_console_reply"   : [(u8, "switches"), (u8, "push_buttons")],
        "on_pneumatic_reply" : [(u8, "pressure")],
        "do_nothing"         : []
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
                                u16(alias="data")),  # Ignore reply
    ],

    # Pneumatic
    "device@49": [
        (CUSTOM,                u8(alias="pressure_input"), "on_pneumatic_reply"),
    ]
})
