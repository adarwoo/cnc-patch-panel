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
   using namespace std::chrono;

   using asx::modbus::command_t;

   /// @brief Period between blinking LED state change
   constexpr auto BLINK_PERIOD = 250ms;

   // Reactor handles
   reactor::Handle react_to_send_beep;
   reactor::Handle react_to_query_console;
   reactor::Handle react_to_query_pneumatic;
   reactor::Handle react_to_set_relay;
   reactor::Handle react_to_console; // External handle

   Relays         relays                 = Relays{0};
   Switches       switches               = Switches{0};
   Key            key                    = Key{Key::None};
   PneumaticCoils coils                  = PneumaticCoils{0};
   ConsoleLeds    console_leds           = ConsoleLeds{0};
   bool           pressure_in            = bool{false};

   CommStatus     relay_comms_status     = CommStatus::down;
   CommStatus     pneu_comms_status      = CommStatus::down;
   CommStatus     console_comms_status   = CommStatus::down;

   // Manage blinking for all LEDs
   auto led_blink_next_change =
      std::array<timer::steady_clock::time_point, 4>{};

   auto led_status =
      std::array<bool, 4>{0};

   constexpr auto time_zero =
      timer::steady_clock::time_point(timer::steady_clock::duration::zero());

   /** Return the current status of the LED */
   bool get_led(uint8_t id) {
      return led_status[id];
   }

   /**
    * Set a virtual LED based on condition
    * @param iomux::led::Id Id of the virtual LED to set
    * @param bool Nominal value
    * @param bool If true, blinks and ignore the nominal value
    */
   void set_led(uint8_t id, bool onoff, bool override) {
      if ( override ) {
         auto now = timer::steady_clock::now();

         if ( led_blink_next_change[id] == time_zero ) {
            led_blink_next_change[id] = now;
            led_status[id] = true;
         } else {
            if ( now - led_blink_next_change[id] > BLINK_PERIOD ) {
               led_status[id] = !led_status[id];
               led_blink_next_change[id] += BLINK_PERIOD;
            }
         }
      } else {
         led_blink_next_change[id] == time_zero;
         led_status[id] = onoff;
      }
   }

   /// @brief When LEDs should change state
   auto blink_next_change =
      std::array<timer::steady_clock::time_point, 4>{};

   /// @brief called every 20ms to sample the console and pneumatic (every 100ms)
   ///   this may be followed by calls to other modbus devices
   /// Calls the console every cycle
   /// 0 1  2  3 4
   /// C CR C CP C | C CR C CP C ... every 100ms
   void on_modbus_cycle() {
      static uint8_t prescaler = 0;

      if ( modbus_master::get_pending_request().is_empty() ) {
         modbus_master::request_to_send(react_to_query_console);

         // Throttle the number of relay
         if ( prescaler == 2 ) {
            modbus_master::request_to_send(react_to_set_relay);
         }

         // Throttle the number of pneumatic packets as this is not a priority
         if ( prescaler == 3 ) {
            // TODO : modbus_master::request_to_send(react_to_query_pneumatic);
         }

         if ( ++prescaler == 5 ) {
            prescaler = 0;
         }
      }
   }

   // -------------------------------------------------------------------------
   // Modbus packets
   // -------------------------------------------------------------------------

   /**
    * Create the modbus beep request packet
    */
   void beep_request() {
      Datagram::pack(console_address);
      Datagram::pack(command_t::write_single_register);
      Datagram::pack<uint16_t>(10); // Beep register (holding)
      Datagram::pack<uint16_t>(2);  // Tone 1, 2 or 3
   }

   /**
    * Create a modbus master payload to query (read and write) the console
    */
   void query_console() {
      // Update the Leds. The reply contains the switches and push button state
      Datagram::pack(console_address);
      Datagram::pack(command_t::custom);
      Datagram::pack(console_leds.lsb);
      Datagram::pack(console_leds.msb);
   }

   /**
    * Update the pneumatic coils and request the pressures readout
    */
   void query_pneumatic() {
      Datagram::pack(pneumatic_relay_address);
      Datagram::pack(command_t::custom);
      Datagram::pack(coils.all);
   }

   /**
    * Set the relays positions
    */
   void set_relay() {
      Datagram::pack(relay_address);
      Datagram::pack(command_t::write_multiple_coils);
      Datagram::pack(0);           // Start address
      Datagram::pack(3);           // Quantity
      Datagram::pack<uint8_t>(1);  // Byte count
      Datagram::pack(relays.all);
   }

   // -------------------------------------------------------------------------
   // Modbus replies
   // -------------------------------------------------------------------------

   /**
    * Process the reply to the custom modbus request
    * Store the selected push button and the switch value and
    *  further delegate the processing to the patch
    */
   void on_console_reply(uint8_t _switches, uint8_t _key) {
      // Store for the handler to process
      switches = static_cast<Switches>(_switches);
      key = static_cast<Key>(_key);

      // Notfiy the external reactor (Stage is 0)
      react_to_console();

      // Set the status to OK
      console_comms_status = CommStatus::ok;
   }

   /**
    * Process the pneumatic custom modbus message reply.
    * Store the pressure switch state
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
   void on_comm_error(uint8_t device_id, asx::modbus::error_t error) {
      auto new_status = (error == asx::modbus::error_t::reply_timeout)
         ? CommStatus::down : CommStatus::error;

      switch (device_id) {
      case console_address:
         console_comms_status = new_status;
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

   /**
    * Request a 'beep' from the console
    */
   void beep() {
      // Request to transmit a beep request
      modbus_master::request_to_send(react_to_send_beep);
   }

   /**
    * Ready the modbus master and the query cycle
    * @param react_on_console_reply
    *        Reactor handle to call once a reply from the console has been
    *        received and the internal caches data are updated
    */
   void init( reactor::Handle react_on_console_reply ) {
      using namespace std::chrono;

      // Register the reactor 'request' handle. First rejected are invoked first

      // Beeps should be first to be handled
      react_to_send_beep            = reactor::bind(beep_request);

      // Pneumatic requests are next
      react_to_query_pneumatic      = reactor::bind(query_pneumatic);
      react_to_query_console        = reactor::bind(query_console);
      react_to_set_relay            = reactor::bind(set_relay);
      react_to_console              = react_on_console_reply;

      // Start the modbus cycle in 2 seconds (to match with when the LEDs turn off)
      using HandlerFn = void (*)(uint8_t, asx::modbus::error_t);

      modbus_master::init(reactor::bind<HandlerFn>(on_comm_error));

      // Start the modbus queries after 2s (relay will take 5)
      reactor::bind(on_modbus_cycle).repeat(2s, 20ms);
   }
}  // namespace modbus