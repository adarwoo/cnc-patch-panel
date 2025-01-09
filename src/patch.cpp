#include <array>

namespace patch {
    // Define a union to combine uint16_t with bitfields
    union Relays {
        uint16_t all;
        struct {
            uint16_t dust : 1;
            uint16_t cool : 1;
            uint16_t spare : 1;
        } bits;
    };

    union Switches {
        uint16_t all;
        struct {
            uint16_t cool : 1;
            uint16_t dust : 1;
            uint16_t release : 1;
            uint16_t door : 1;
        } bits;
    };

    // Define a union to combine uint16_t with bitfields
    union PneumaticCoils {
        uint16_t all;
        struct {
            uint16_t air_blast : 1;
            uint16_t chuck : 1;
            uint16_t spindle_clean : 1;
            uint16_t door_pull : 1;
            uint16_t chuck_open : 1;
            uint16_t door_push : 1;
        } bits;
    };

    // Define a union to combine uint16_t with bitfields
    union ConsoleLeds {
        uint16_t all;
        struct {
            uint16_t start : 1;
            uint16_t stop : 1;
            uint16_t home : 1;
            uint16_t goto0 : 1;
            uint16_t park : 1;
            uint16_t change_tool : 1;
        } bits;
    };

    // Define a union to combine uint16_t with bitfields
    union LedFB {
        uint16_t all;
        struct {
            uint16_t mso_in0 : 1; // Key pressed
            uint16_t mso_in1 : 1; // Relay ? 1-2
            uint16_t mso_in2 : 1; // door state machine ?
            uint16_t mso_in3 : 1;
            uint16_t mso_in4 : 1;
            uint16_t mso_in5 : 1;
            uint16_t mso_in6 : 1;
            uint16_t mso_in7 : 1;
            uint16_t mso_in8 : 1;
            uint16_t mso_in9 : 1;
            uint16_t mso_in10 : 1;
            uint16_t mso_in11 : 1;
            uint16_t spindle_on : 1;
            uint16_t door_up : 1;
            uint16_t door_down : 1;
            uint16_t error : 1;
        } bits;
    };

    struct OutputToMasso : public Pin
    {
        Coil(PinDef _pin) : Pin(_pin) {
           init(dir_t::out);
       }
    };

    enum MassoInputPins : uint8_t {
        start_key,
        stop_key,
        home_key,
        goto0_key,
        park_key,
        tool_change_key,
        door_key,
        fn1_key,
        fn2_key,
        fn3_key,
        fn4_key,
        fn5_key,
        fn6_key
    };

   std::array<OutputToMasso, 13> outputs_to_masso = {
       PinDef{MSO_OUT0  },
       PinDef{MSO_OUT1  },
       PinDef{MSO_OUT2  },
       PinDef{MSO_OUT3  },
       PinDef{MSO_OUT4  },
       PinDef{MSO_OUT5  },
       PinDef{MSO_OUT6  },
       PinDef{MSO_OUT7  },
       PinDef{MSO_OUT8  },
       PinDef{MSO_OUT9  },
       PinDef{MSO_OUT10 },
       PinDef{MSO_OUT11 },
       PinDef{MSO_OUT12 },
   };    

    // All LEDs on to start with
    auto led_fb = LedFB{0xffff};

    // System pneumatic coils
    auto pneumatic_coils = PneumaticCoils{0};

    // State of the relays
    auto relays = Relays{0};

    // Console LEDs
    auto console_leds = ConsoleLeds{0};

    auto iomux_in = PCA9555(0);
    auto iomux_led = PCA9555(2);

    auto pressure_in = bool{false};

    auto error_flag = bool{false};

    auto relay_needs_update = bool{false};

    auto last_know_switches = Switches{0};

    struct InitPCA {
        auto operator()() {
            return make_transition_table(
                * "idle"_s          + event<start>     / [] {iomux_led.set_value(0xffff, react_on_i2c_ready); }       = "init1"_s
                , "init1"_s         + event<i2c_ready> / [] {iomux_led.set_dir(0, react_on_i2c_ready); }              = "init2"_s
                , "init2"_s         + event<i2c_ready> / [] {react_on_poll.repeat(2ms); }                             = "wait_for_poll"_s
                , "wait_for_poll"_s + event<polling>   / [] {iomux_in.read(react_on_i2c_ready, react_on_i2c_ready); } = "set_leds"_s
                , "set_leds"_s      + event<i2c_ready> / [] {iomux_led.set_value(led_fb.all, react_on_i2c_ready); }   = "wait_for_poll"_s
            );
        }
    };

    /// @brief Called every 250ms
    void on_drive_error_led() {
        if ( error_flag ) {
            led_fb.error = ! led_fb.error;
        }
    }

    void on_ready_query_modbus() {
        // Make sure the SM is ok
        if ( not sm.state(idle) ) {
            // Postpone!
            return;
        }
    }

    /// @brief called every 25ms to sample the console, this may be followed by calls to other modbus devices
    void on_cycle() {
        // Make sure the modbus bus is available
        if ( sm.state(idle) ) {
            // Query the console
            Datagram::pack(modbus:command_t::custom);
            Datagram::pack(console_leds.all);

            sm.process_event(transmit{});
        }
    }

    void on_console_reply(uint8_t switches, uint8_t key) {
        // Write the key to masso
        for ( uint8_t i=0; i<outputs_to_masso.size(); ++i ) {
            outputs_to_masso.set( i == keys );
        }

        // Write the switches to the relay and masso
        auto latest_sw = Switches{Switches};

        if ( last_know_switches.bits.cool != latest_sw.bits.cool || last_know_switches.bits.dust != latest_sw.bits.dust ) {
            relay_needs_update = true;
            relays.bits.dust = latest_sw.bits.dust;
            relays.bits.cool = latest_sw.bits.cool;
        }

        // Query the pneumatic
        Datagram::pack(modbus:command_t::custom);
        Datagram::pack(pneumatic_coils.all);

        sm.process_event(transmit{});
    }

    void on_pneumatic_reply() {
        if ( relay_needs_update ) {
            // Update the relay
            Datagram::pack(modbus:command_t::write_multiple_coils);
            Datagram::pack(0); // Start address
            Datagram::pack(3); // Quantity
            Datagram::pack<uint8_t>(2); // Byte count (2*N)
            Datagram::pack(relays);
            Datagram::pack(pneumatic_coils.all);

            sm.process_event(transmit{});
        }
    }
}