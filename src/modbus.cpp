#include <array>
#include <cstdint>

#include <asx/ioport.hpp>
#include <asx/reactor.hpp>

#include <trace.h>

#include "conf_board.h"
#include "conf_modbus.hpp"
#include "datagram.hpp"


namespace modbus {
   using namespace asx;

   // Reactor handles
   reactor::Handle react_to_query_console;
   reactor::Handle react_to_query_pneumatic;
   reactor::Handle react_to_set_relay;

   // Local variables
   auto pneumatic_coils    = PneumaticCoils{0}; // System pneumatic coils
   auto relays             = Relays{0};         // State of the relays
   auto console_leds       = ConsoleLeds{0};    // Console LEDs
   auto pressure_in        = bool{false};       // State of the pressure
   auto error_flag         = bool{false};       // Error detected
   auto last_know_switches = Switches{0};       // Current switch pushed

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
      #if 0
      static auto current_relays_value = Relays{0};
      static auto current_iso_value = IsolatedOutputs{0};
      #endif

      // Request bus to transmit to the console
      TRACE_INFO(PATCH, "Request to send %u", static_cast<uint8_t>(react_to_query_console));

      if ( ++prescaler == 4 )
      {
         patch::modbus_master::request_to_send(react_to_query_console);
         prescaler = 0;
      }


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
      static uint16_t chaser = 15;
      // Update the Leds. The reply contains the switches and push button state
      Datagram::pack(console_address);
      Datagram::pack(modbus::command_t::custom);
      Datagram::pack(chaser << 1);
      if ( chaser == 0b100000 ) chaser=1;
   }

   void query_pneumatic() {
      // Update the pneumatic coils and get the pressure readout
      Datagram::pack(pneumatic_relay_address);
      Datagram::pack(modbus::command_t::custom);
      Datagram::pack(pneumatic_coils.all);
   }

   void set_relay() {
      // Update the relay
      Datagram::pack(relay_address);
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
      i2c::Master::request(react_on_init_i2c_devices);
      i2c_sequencer.process_event(start{});

      // Start the modbus cycle in 2 seconds (to match with when the LEDs turn off)
      modbus_master::init();
      reactor::bind(on_modbus_cycle).repeat(2s, 100ms);
   }
}  // namespace modbus