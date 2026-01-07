#include <array>
#include <cstdint>

#include <ulog.h>
#include <asx/ioport.hpp>
#include <asx/reactor.hpp>

#include "conf_board.h"
#include "conf_modbus.hpp"
#include "datagram.hpp"
#include "modbus.hpp"


namespace modbus {
   using namespace asx;
   using namespace asx::chrono;

   using asx::modbus::command_t;

   /// @brief Period between blinking LED state change
   constexpr auto BLINK_PERIOD = std::chrono::milliseconds(250);

   // Reactor handles
   reactor::Handle react_to_send_beep;
   reactor::Handle react_to_query_console;
   reactor::Handle react_to_get_set_pneumatic;
   reactor::Handle react_to_set_relay;
   reactor::Handle react_to_console; // External handle

   Relays         relays                 = Relays{0b001}; // Default compressor ON
   Switches       switches               = Switches{0};
   Key            key                    = Key{Key::None};
   PneumaticCoils coils                  = PneumaticCoils{0};
   ConsoleLeds    console_leds           = ConsoleLeds{0};
   bool           pressure_detected      = bool{false};
   bool           water_pump_alarm       = bool{false};

   CommStatus     relay_comms_status     = CommStatus::down;
   CommStatus     pneu_comms_status      = CommStatus::down;
   CommStatus     console_comms_status   = CommStatus::down;

   // Manage blinking for all LEDs
   auto led_blink_next_change =
      std::array<steady_clock::time_point, 4>{};

   auto led_status =
      std::array<bool, 4>{0};

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
         // Blink it!
         auto now = chrono::steady_clock::now();

         if ( led_blink_next_change[id] == chrono::time_zero ) {
            led_blink_next_change[id] = now + BLINK_PERIOD;
            led_status[id] = true; // Set to start with (reacts immediately)
         } else {
            if ( led_blink_next_change[id] <= now ) {
               led_status[id] = !led_status[id];
               led_blink_next_change[id] = now + BLINK_PERIOD;
            }
         }
      } else {
         led_blink_next_change[id] == chrono::time_zero;
         led_status[id] = onoff;
      }
   }

   /// @brief When LEDs should change state
   auto blink_next_change =
      std::array<steady_clock::time_point, 4>{};

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
            ULOG_DEBUG2("Queuing relay update: 0x{:02x}", relays.all);
            modbus_master::request_to_send(react_to_set_relay);
         }

         // Pneumatic read every 5 cycles (100ms * 5 = 500ms)
         if ( prescaler == 4 ) {
            ULOG_DEBUG2("Controlling pneumatic coils: 0x{:02x}", coils.all);
            modbus_master::request_to_send(react_to_get_set_pneumatic);
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
      Datagram::pack(console_leds.msb);
      Datagram::pack(console_leds.lsb);
   }

   /**
    * Update the pneumatic coils and request the pressures readout
    */
   void get_set_pneumatic() {
      Datagram::pack(pneumatic_relay_address);
      Datagram::pack(command_t::custom);
      // TODO Datagram::pack(coils.all);
      Datagram::pack(uint8_t(0)); // Currently no coil is set
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
      auto old_switches = switches;
      auto old_key = key;
      switches = static_cast<Switches>(_switches);
      key = static_cast<Key>(_key);

      // Log changes in switches or key state
      if (old_switches.all != switches.all || old_key != key) {
         ULOG_TRACE("Console: switches=0x{:02x} key={}", switches.all, static_cast<uint8_t>(key));
      }

      // Notfiy the external reactor (Stage is 0)
      react_to_console();

      // Set the status to OK
      console_comms_status = CommStatus::ok;
   }

   /**
    * Process the pneumatic custom modbus message reply.
    * Store the pressure switch state
    */
   void on_pneumatic_reply(uint8_t value) {
      auto pressure_switch_state = static_cast<bool>(value & 0x01);
      auto water_pump_state = static_cast<bool>(value & 0x02);

      if (pressure_detected != pressure_switch_state) {
         pressure_detected = pressure_switch_state;
         ULOG_INFO("Pneumatic pressure change: {}", pressure_detected);
      }

      if (water_pump_alarm != water_pump_state) {
         water_pump_alarm = water_pump_state;
         ULOG_WARN("Pneumatic water pump alarm change: {}", water_pump_alarm);
      }

      pneu_comms_status = CommStatus::ok;
   }

   /**
    * Called when the pneumatic set command was replied OK
    */
   void on_pneumatic_set_reply() {
      ULOG_DEBUG2("Pneumatic set confirmed: coils=0x{:02x}", coils.all);

      // Set the status to OK
      pneu_comms_status = CommStatus::ok;
   }

   /**
    * Called when the relay replied OK
    */
   void on_relay_reply() {
      ULOG_DEBUG2("Relay update confirmed: 0x{:02x}", relays.all);

      // Set the status to OK
      relay_comms_status = CommStatus::ok;
   }

   /**
    * Called when an error was detected
    */
   void on_comm_error(uint8_t device_id, asx::modbus::error_t error) {
      auto new_status = (error == asx::modbus::error_t::reply_timeout)
         ? CommStatus::down : CommStatus::error;

      constexpr auto device_name = [](uint8_t device_id) constexpr -> const char* {
         switch (device_id) {
         case console_address:         return "Console";
         case relay_address:           return "Relay";
         case pneumatic_relay_address: return "Pneumatic";
         default:                      return "Unknown";
         }
      };

      constexpr auto status_name = [](CommStatus status) noexcept {
         switch(status) {
            case CommStatus::error: return "error";
            case CommStatus::down:  return "down";
            case CommStatus::ok:    return "ok";
            default:                return "unknown";
         }
      };

      ULOG_WARN("Modbus error device {} => {}", device_name(device_id), status_name(new_status));

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
      ULOG_INFO("Beep requested");
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
      react_to_get_set_pneumatic    = reactor::bind(get_set_pneumatic);
      react_to_query_console        = reactor::bind(query_console);
      react_to_set_relay            = reactor::bind(set_relay);
      react_to_console              = react_on_console_reply;

      // Start the modbus cycle in 2 seconds (to match with when the LEDs turn off)
      using HandlerFn = void (*)(uint8_t, asx::modbus::error_t);

      modbus_master::init(reactor::bind<HandlerFn>(on_comm_error));

      // Start the modbus queries after 2s (relay will take 5)
      reactor::bind(on_modbus_cycle).repeat(2s, 100ms);
   }
}  // namespace modbus