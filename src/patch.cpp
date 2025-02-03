#include <array>
#include <cstdint>

#include <boost/sml.hpp>

#include <asx/ioport.hpp>
#include <asx/pca9555.hpp>
#include <asx/reactor.hpp>

#include "conf_board.h"
#include "conf_modbus.hpp"
#include "datagram.hpp"

#include "patch.hpp"


namespace patch {
   using namespace asx::ioport;
   using namespace asx::i2c;
   using namespace asx;


   // Reactor handles
   reactor::Handle react_on_poll;
   reactor::Handle react_to_query_console;
   reactor::Handle react_to_query_pneumatic;
   reactor::Handle react_to_set_relay;
   reactor::Handle react_to_set_isolated_outputs;

   // Local variables
   auto led_fb             = LedFB{0xffff};     // All local LEDs on to start with
   auto pneumatic_coils    = PneumaticCoils{0}; // System pneumatic coils
   auto relays             = Relays{0};         // State of the relays
   auto isolated_outputs   = IsolatedOutputs{0};// State of the isolated outputs
   auto console_leds       = ConsoleLeds{0};    // Console LEDs
   auto iomux_io           = PCA9555(0);        // IOMux 0 For input pins
   auto iomux_led          = PCA9555(2);        // IOMux 2 For LEDs
   auto pressure_in        = bool{false};       // State of the pressure
   auto error_flag         = bool{false};       // Error detected
   auto last_know_switches = Switches{0};       // Current switch pushed

   // Handlers
   void on_i2c_ready(status_code_t code);

   // i2c state machine
   struct InitPCA {
      auto operator()() {
         using namespace boost::sml;
         using namespace std::chrono;

         return make_transition_table(
            *"idle"_s          + event<start>     / [] { iomux_led.set_value(0xffff, on_i2c_ready);     } = "init_led1"_s,
             "init_led1"_s     + event<i2c_ready> / [] { iomux_led.set_dir(0, on_i2c_ready);            } = "init_led2"_s,
             "init_led2"_s     + event<i2c_ready> / [] { iomux_io.set_value(0, on_i2c_ready);           } = "init_io1"_s,
             "init_io1"_s      + event<i2c_ready> / [] { iomux_io.set_dir(IO_MASK, on_i2c_ready);       } = "init_io2"_s,
             "init_io2"_s      + event<i2c_ready> / [] { react_on_poll.repeat(2ms);                     } = "wait_for_poll"_s,
             "wait_for_poll"_s + event<polling>   / [] { iomux_io.read<1>(on_i2c_ready);                } = "set_leds"_s,
             "set_leds"_s      + event<i2c_ready> / [] { iomux_led.set_value(led_fb.all, on_i2c_ready); } = "wait_for_poll"_s
         );
      }
   };

   boost::sml::sm<InitPCA> i2c_sequencer;

   void on_i2c_ready(status_code_t code) {
      alert_and_stop_if(code != status_code_t::STATUS_OK);
      i2c_sequencer.process_event(i2c_ready{});
   }

   void on_poll_input() {
      i2c_sequencer.process_event(polling{});
   }

   /// @brief Called every 250ms
   void on_drive_error_led() {
      if (error_flag) {
         led_fb.bits.general_error = !led_fb.bits.general_error;
      }
   }

   /// @brief called every 25ms to sample the console and pneumatic (every 100ms)
   ///   this may be followed by calls to other modbus devices
   void on_modbus_cycle() {
      static uint8_t prescaler = 0;
      static auto current_relays_value = Relays{0};
      static auto current_iso_value = IsolatedOutputs{0};

      // Request bus to transmit to the console
      trace("Request to send %u", static_cast<uint8_t>(react_to_query_console));

      patch::modbus_master::request_to_send(react_to_query_console);
#if 0
      if ( ++prescaler == 4 ) {
         patch::modbus_master::request_to_send(react_to_query_pneumatic);
      }

      if ( current_relays_value.all != relays.all ) {
         patch::modbus_master::request_to_send(react_to_set_relay);
         current_relays_value.all = relays.all;
      }

      if ( current_iso_value.all != isolated_outputs.all ) {
         patch::modbus_master::request_to_send(react_to_set_isolated_outputs);
         current_iso_value.all = isolated_outputs.all;
      }
#endif
   }

   /// Create a modbus master payload to query (read and write) the console
   void query_console() {
      // Update the Leds. The reply contains the switches and push button state
      Datagram::pack(modbus::command_t::custom);
      Datagram::pack(led_fb.all);
   }

   void query_pneumatic() {
      // Update the pneumatic coils and get the pressure readout
      Datagram::pack(modbus::command_t::custom);
      Datagram::pack(pneumatic_coils.all);
   }

   void set_relay() {
      // Update the relay
      Datagram::pack(modbus::command_t::write_multiple_coils);
      Datagram::pack(0);           // Start address
      Datagram::pack(3);           // Quantity
      Datagram::pack<uint8_t>(2);  // Byte count (2*N)
      Datagram::pack(relays.all);
   }

   void set_isolated_outputs() {
      // Update the relay
      Datagram::pack(modbus::command_t::write_multiple_coils);
      Datagram::pack(0);           // Start address
      Datagram::pack(3);           // Quantity
      Datagram::pack<uint8_t>(2);  // Byte count (2*N)
      Datagram::pack(isolated_outputs.all);
   }

   void on_console_reply(uint8_t switches, uint8_t key) {
      // Write the key to masso
      led_fb.bits.console_key_code = key % 7;
      led_fb.bits.console_shift = (key > 7);

      // Write the switches to the relay and masso
      auto latest_sw = Switches{switches};

      led_fb.bits.override_door = latest_sw.bits.door;
      led_fb.bits.dust          = latest_sw.bits.dust;
      led_fb.bits.cooling       = latest_sw.bits.cool;
      led_fb.bits.free_axis     = latest_sw.bits.release;
   }

   /**
    * Called when the modbus
    */
   void on_pneumatic_reply(uint8_t switch_state) {
      pressure_in = switch_state;
   }

   void init() {
      using namespace std::chrono;

      react_on_poll                 = reactor::bind(on_poll_input);
      react_to_query_pneumatic      = reactor::bind(query_pneumatic,      reactor::prio::high);
      react_to_query_console        = reactor::bind(query_console,        reactor::prio::high);
      react_to_set_relay            = reactor::bind(set_relay,            reactor::prio::high);
      react_to_set_isolated_outputs = reactor::bind(set_isolated_outputs, reactor::prio::high);

      i2c::Master::init(400_KHz);
      i2c_sequencer.process_event(start{});

      // Start the modbus cycle in 2 seconds (to match with when the LEDs turn off)
      modbus_master::init();
      reactor::bind(on_modbus_cycle).repeat(2s, 100ms);
   }
}  // namespace patch