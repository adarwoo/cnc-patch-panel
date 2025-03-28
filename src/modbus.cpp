#include <array>
#include <cstdint>

#include <asx/ioport.hpp>
#include <asx/reactor.hpp>

#include <trace.h>

#include "conf_board.h"
#include "conf_modbus.hpp"
#include "datagram.hpp"

#include "modbus.hpp"


namespace modbus {
   using namespace asx;
   using asx::modbus::command_t;

   // Reactor handles
   reactor::Handle react_to_send_beep;
   reactor::Handle react_to_query_console;
   reactor::Handle react_to_query_pneumatic;
   reactor::Handle react_to_set_relay;
   reactor::Handle react_to_console; // External handle

   /// @brief called every 20ms to sample the console and pneumatic (every 100ms)
   ///   this may be followed by calls to other modbus devices
   /// Calls the console every cycle
   /// 0 1  2  3 4 
   /// C CR C CP C | C CR C CP C ... every 100ms
   void on_modbus_cycle() {
      static uint8_t prescaler = 0;
      static auto current_relays_value = Relays{0};

      modbus_master::request_to_send(react_to_query_console);

      // Throttle the number of relay
      if ( prescaler == 1 and current_relays_value.all != relays.all ) {
         modbus_master::request_to_send(react_to_set_relay);
         current_relays_value.all = relays.all;
      }

      // Throttle the number of pneumatic packets as this is not a priority
      if ( prescaler == 3 ) {
         modbus_master::request_to_send(react_to_query_pneumatic);
         prescaler = 0;
      }

      if ( ++prescaler == 5 ) {
         prescaler = 0;
      }
   }

   /// Beep
   void beep_request() {
      Datagram::pack(console_address);
      Datagram::pack(command_t::write_single_register);
      Datagram::pack<uint16_t>(10); // Beep register (holding)
      Datagram::pack<uint16_t>(2);  // Tone 1, 2 or 3
   }

   /// Create a modbus master payload to query (read and write) the console
   void query_console() {
      // Update the Leds. The reply contains the switches and push button state
      Datagram::pack(console_address);
      Datagram::pack(command_t::custom);
      Datagram::pack(console_leds.all);
   }

   void query_pneumatic() {
      // Update the pneumatic coils and get the pressure readout
      Datagram::pack(pneumatic_relay_address);
      Datagram::pack(command_t::custom);
      Datagram::pack(coils.all);
   }

   void set_relay() {
      // Update the relay
      Datagram::pack(relay_address);
      Datagram::pack(command_t::write_multiple_coils);
      Datagram::pack(0);           // Start address
      Datagram::pack(3);           // Quantity
      Datagram::pack<uint8_t>(2);  // Byte count (2*N)
      Datagram::pack(relays.all);
   }

   void on_console_reply(uint8_t _switches, uint8_t _key) {
      // Store for the handler to process
      switches = static_cast<Switches>(_switches);
      key = static_cast<Key>(_key);

      // Notfiy the external reactor
      react_to_console.notify();

      // Set the status to OK
      console_comms_status = CommStatus::ok;
   }

   /**
    * Called when the modbus
    */
   void on_pneumatic_reply(uint8_t switch_state) {
      pressure_in = switch_state;

      // Set the status to OK
      pneu_comms_status = CommStatus::ok;
   }

   /**
    * Called when the relay replied OK
    */
   void on_relay_reply() {
      // Set the status to OK
      relay_comms_status = CommStatus::ok;
   }

   /**
    * Called when an error was detected
    */
   void on_comm_error(uint8_t device_id, bool comm_lost) {
      auto new_status = comm_lost ? CommStatus::down : CommStatus::error;

      switch (device_id) {
      case console_address:
         relay_comms_status = new_status;
         break;
      case pneumatic_relay_address:
         pneu_comms_status = new_status;
         break;
      case relay_address:
         relay_comms_status = new_status;
         break;
      
      default:
         break;
      }
   }

   void init( reactor::Handle update ) {
      using namespace std::chrono;

      // Register the reactor 'request' handle. First rejected are invoked first

      // Beeps should be top priority
      react_to_send_beep            = reactor::bind(beep_request,         reactor::prio::high);

      // Pneumatic requests are next
      react_to_query_pneumatic      = reactor::bind(query_pneumatic,      reactor::prio::high);
      react_to_query_console        = reactor::bind(query_console,        reactor::prio::high);
      react_to_set_relay            = reactor::bind(set_relay,            reactor::prio::high);
      react_to_console              = update;

      // Start the modbus cycle in 2 seconds (to match with when the LEDs turn off)
      modbus_master::init(reactor::bind(on_comm_error));

      // Start the modbus queries after 2s
      reactor::bind(on_modbus_cycle).repeat(2s, 20ms);
   }

   void beep() {
      // Request to transmit a beep request
      modbus_master::request_to_send(react_to_send_beep);
   }
}  // namespace modbus