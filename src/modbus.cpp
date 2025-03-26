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
   reactor::Handle react_to_query_console;
   reactor::Handle react_to_query_pneumatic;
   reactor::Handle react_to_set_relay;
   reactor::Handle react_to_console; // External handle

   /// @brief called every 25ms to sample the console and pneumatic (every 100ms)
   ///   this may be followed by calls to other modbus devices
   void on_modbus_cycle() {
      static uint8_t prescaler = 0;
      #if 0
      static auto current_relays_value = Relays{0};
      #endif

      // Request bus to transmit to the console
      TRACE_INFO(PATCH, "Request to send %u", static_cast<uint8_t>(react_to_query_console));

      if ( ++prescaler == 4 )
      {
         modbus_master::request_to_send(react_to_query_console);
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
#endif
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

      react_to_query_pneumatic      = reactor::bind(query_pneumatic,      reactor::prio::high);
      react_to_query_console        = reactor::bind(query_console,        reactor::prio::high);
      react_to_set_relay            = reactor::bind(set_relay,            reactor::prio::high);
      react_to_console              = update;

      // Start the modbus cycle in 2 seconds (to match with when the LEDs turn off)
      modbus_master::init(reactor::bind(on_comm_error));

      // Start the modbus queries after 2s
      reactor::bind(on_modbus_cycle).repeat(2s, 100ms);
   }
}  // namespace modbus