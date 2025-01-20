#include <array>
#include <asx/ioport.hpp>
#include <asx/pca9555.hpp>
#include <asx/reactor.hpp>
#include <boost/sml.hpp>
#include <cstdint>

#include "conf_board.h"
#include "datagram.hpp"

#include "patch.hpp"


namespace patch {
   using namespace asx::ioport;
   using namespace asx::i2c;
   using namespace asx;

   auto led_fb             = LedFB{0xffff};     // All LEDs on to start with
   auto pneumatic_coils    = PneumaticCoils{0}; // System pneumatic coils
   auto relays             = Relays{0};         // State of the relays
   auto console_leds       = ConsoleLeds{0};    // Console LEDs
   auto iomux_in           = PCA9555(0);        // IOMux 0 For input pins
   auto iomux_led          = PCA9555(2);        // IOMux 2 For LEDs
   auto pressure_in        = bool{false};       // State of the pressure
   auto error_flag         = bool{false};       // Error detected
   auto relay_needs_update = bool{false};       // True if the relay should be changed
   auto last_know_switches = Switches{0};       // Current switch pushed

   // Reactor handles
   reactor::Handle react_on_i2c_ready;
   reactor::Handle react_on_poll;

   struct InitPCA {
      auto operator()() {
         using namespace boost::sml;
         using namespace std::chrono;

         return make_transition_table(
            *"idle"_s          + event<start>     / [] { iomux_led.set_value(0xffff, react_on_i2c_ready);       } = "init1"_s,
             "init1"_s         + event<i2c_ready> / [] { iomux_led.set_dir(0, react_on_i2c_ready);              } = "init2"_s,
             "init2"_s         + event<i2c_ready> / [] { react_on_poll.repeat(2ms);                             } = "wait_for_poll"_s,
             "wait_for_poll"_s + event<polling>   / [] { iomux_in.read(react_on_i2c_ready);                     } = "set_leds"_s,
             "set_leds"_s      + event<i2c_ready> / [] { iomux_led.set_value(led_fb.all, react_on_i2c_ready);   } = "wait_for_poll"_s
         );
      }
   };

   boost::sml::sm<InitPCA> i2c_sequencer;

   auto on_i2c_ready(status_code_t code) {
      alert_and_stop_if(code != status_code_t::STATUS_OK);
      i2c_sequencer.process_event(i2c_ready{});
   }

   auto on_poll_input() {
      i2c_sequencer.process_event(polling{});
   }

   /// @brief Called every 250ms
   void on_drive_error_led() {
      if (error_flag) {
         led_fb.bits.error = !led_fb.bits.error;
      }
   }

   /// @brief called every 25ms to sample the console, this may be followed by calls to other modbus devices
   void on_modbus_cycle() {
      // Query the console
      //Datagram::pack(modbus : command_t::custom);
      //Datagram::pack(console_leds.all);
   }

   void on_console_reply(uint8_t switches, uint8_t key) {
      // Write the key to masso
      for (uint8_t i = 0; i < outputs_to_masso.size(); ++i) {
         ioport::Pin(outputs_to_masso[i]).set(i == (key-1));
      }

      // Write the switches to the relay and masso
      auto latest_sw = Switches{switches};

      if (last_know_switches.bits.cool != latest_sw.bits.cool || last_know_switches.bits.dust != latest_sw.bits.dust) {
         relay_needs_update = true;
         relays.bits.dust = latest_sw.bits.dust;
         relays.bits.cool = latest_sw.bits.cool;
      }
   }

   /**
    * Called when the modbus
    */
   void on_pneumatic_reply() {
      #if 0
      if (relay_needs_update) {
         // Update the relay
         Datagram::pack(modbus::command_t::write_multiple_coils);
         Datagram::pack(0);           // Start address
         Datagram::pack(3);           // Quantity
         Datagram::pack<uint8_t>(2);  // Byte count (2*N)
         Datagram::pack(relays);
         Datagram::pack(pneumatic_coils.all);
      }
      #endif
   }

   void init() {
      react_on_i2c_ready = reactor::bind(on_i2c_ready);
      react_on_poll = reactor::bind(on_poll_input);

      i2c::Master::init(400_KHz);
      i2c_sequencer.process_event(start{});
   }
}  // namespace patch