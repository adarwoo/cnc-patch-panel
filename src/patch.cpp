#include <array>
#include <cstdint>

#include <asx/ioport.hpp>
#include <asx/reactor.hpp>
#include <trace.h>

#include "conf_board.h"

#include "patch.hpp"
#include "iomux.hpp"
#include "modbus.hpp"

namespace patch {
   using namespace asx::ioport;
   using namespace asx;

   /**
    * Map comm status to LED status.
    * No comm - LED is off
    * Errors - LED blinks
    * OK - LED is on
    */
   iomux::led::Status to_led_status(modbus::CommStatus status) {
      auto retval = iomux::led::Status::off;

      if ( status != modbus::CommStatus::down ) {
         retval = (status == modbus::CommStatus::ok) ? iomux::led::Status::on : iomux::led::Status::blinks;
      }

      return retval;
   }

   // Helper to set the led for the clean
   iomux::led::Status get_clean_led_status() {
      auto retval = iomux::led::Status::off;

      if ( modbus::coils.air_blast ) {
         retval = iomux::led::Status::on;
      }

      if ( modbus::coils.spindle_clean ) {
         return iomux::led::Status::blinks;
      }
      
      return retval;
   }

   /**
    * Called by the i2c with the value 0 once all the i2c IO Mux are in
    * The function splits the operation to allow the Modbus operation
    * to take place in parallel limiting the jitter.
    */
   static void on_patch(uint8_t stage) {
      // Keep the state of the beep to detect the edge
      static bool beep = false;

      switch ( stage ) {
      case 0:
         // Sync pneumatics
         iomux::outputs.air_pressure_low_alarm = modbus::pressure_in;
         modbus::coils.chuck = iomux::inputs.chuck_pressure;

         //
         // Drive MPU GPIOs
         //
         Pin(ISO_OUT_ES                ).set( iomux::inputs.es );
         Pin(ISO_OUT_TOWER_LIGHT_RED   ).set( iomux::inputs.tower_red );
         Pin(ISO_OUT_TOWER_LIGHT_YELLOW).set( iomux::inputs.tower_yellow );
         Pin(ISO_OUT_TOWER_LIGHT_GREEN ).set( iomux::inputs.tower_green );
         Pin(ISO_OUT_LASER_CROSS       ).set( iomux::inputs.laser_crossair );
         Pin(ISO_OUT_CAMERA_LIGHT      ).set( iomux::inputs.camera_light );

         // Ask to plan the next step
         reactor::yield(1);
         break;

      case 1:
         // OC Outputs
         iomux::led::set(iomux::led::Id::tower_red,     iomux::inputs.tower_red);
         iomux::led::set(iomux::led::Id::tower_yellow,  iomux::inputs.tower_yellow);
         iomux::led::set(iomux::led::Id::tower_green,   iomux::inputs.tower_green);
         iomux::led::set(iomux::led::Id::laser_cross,   iomux::inputs.laser_crossair);
         iomux::led::set(iomux::led::Id::cam_light,     iomux::inputs.camera_light);
         iomux::led::set(
            iomux::led::Id::release_steppers,           *Pin(ISO_OUT_RELEASE_STEPPER)
         );

         // Ask to plan the next step
         reactor::yield(2);
         break;

      case 2:
         // Air LEDs
         iomux::led::set(iomux::led::Id::clean,         get_clean_led_status());
         iomux::led::set(iomux::led::Id::low_pressure,  modbus::pressure_in);
         iomux::led::set(iomux::led::Id::chuck,         iomux::inputs.chuck_pressure);

         // Modbus comms LEDs
         iomux::led::set(iomux::led::Id::console,       to_led_status(modbus::console_comms_status));
         iomux::led::set(iomux::led::Id::pneumatic_hub, to_led_status(modbus::pneu_comms_status));
         iomux::led::set(iomux::led::Id::relay,         to_led_status(modbus::relay_comms_status));

         // Ask to plan the next step
         reactor::yield(3);
         break;

      case 3:
      default:
         // Door
         iomux::led::set(iomux::led::Id::door_closing,  false); // TODO
         iomux::led::set(iomux::led::Id::door_opening,  false); // TODO

         //
         // Drive Masso inputs (our outputs) which are not keys
         //
         iomux::outputs.door_sensor_input = modbus::switches.door || iomux::inputs.door_is_down;

         //
         // Process the switches' Leds
         //
         modbus::console_leds.door    = iomux::virtual_leds.door;
         modbus::console_leds.cool    = iomux::virtual_leds.cool;
         modbus::console_leds.dust    = iomux::virtual_leds.dust;
         modbus::console_leds.release = iomux::virtual_leds.release;

         //
         // Sounder (for door alarm)
         //
         modbus::console_leds.sounder = iomux::inputs.sounder;

         // Detect a change in the beep
         if ( beep != iomux::inputs.touch_screen_beep ) {
            beep = iomux::inputs.touch_screen_beep;

            // Request modbus beep in rising edge only
            if ( beep ) {
               modbus::beep();
            }
         }

         break;
      }
   }

   /**
    * Set a virtual LED based on condition
    * @param iomux::led::Id Id of the virtual LED to set
    * @param bool Nominal value
    * @param bool If true, blinks and ignore the nominal value
    */ 
   inline void set_virtual(iomux::led::Id id, bool onoff, bool override) {
      if ( override ) {
         iomux::led::blink(id);
      } else {
         iomux::led::set(id, onoff);
      }
   }
   
   static void on_modbus_console_reply(uint8_t stage) {
      switch (stage) {
      case 0: {
         //
         // Map the keys to the output
         //
         iomux::Outputs v{0};

         switch (modbus::key) {
         case modbus::Key::Start:  v.button_start = 1;         break;
         case modbus::Key::Stop:   v.button_stop = 1;          break;
         case modbus::Key::Homing: v.button_home = 1;          break;
         case modbus::Key::Goto0:  v.button_go_to_home = 1;    break;
         case modbus::Key::Park:   v.button_go_to_parking = 1; break;
         case modbus::Key::Chuck:  v.chuck_clamp_unclamp = 1;  break;
         case modbus::Key::Door:   v.door_open_close = 1;      break;
         case modbus::Key::P1:     v.autoload_g_code1 = 1;     break;
         case modbus::Key::P2:     v.autoload_g_code1 = 1;     break;
         case modbus::Key::P3:     v.autoload_g_code1 = 1;     break;
         default:
            break;
         }

         iomux::outputs = v; // Reactor guarantees atomicity

         //
         //  Map the switches
         //

         // If the release switch is on - set the matching OC output
         Pin(ISO_OUT_RELEASE_STEPPER).set( modbus::switches.release );
         
         // Set the virtual LED mode
         iomux::led::set(iomux::led::Id::virtual_release, iomux::led::Status::blinks);
         
         // Copy the virtual LED value to the modbus payload
         modbus::console_leds.release = iomux::led::get(iomux::led::Id::virtual_release);

         // Next stage
         reactor::yield(1);
         break;
      }

      case 1:
         // Set the door LED (set the virtual LED to manage blinking then assign)
         set_virtual(
            iomux::led::Id::virtual_door,   // Virtual LED to set
            not iomux::inputs.door_is_down, // Condition true (LED is on)
            modbus::switches.door           // Condition blink
         );

         // Set the coolant console LED
         set_virtual(
            iomux::led::Id::virtual_cool,   // Virtual LED to set
            iomux::inputs.spindle_is_on,    // Condition true (LED is on)
            modbus::switches.cool           // Condition blink
         );

         // Set the dust
         set_virtual(
            iomux::led::Id::virtual_dust,   // Virtual LED to set
            iomux::inputs.spindle_is_on,    // Condition true (LED is on)
            modbus::switches.dust           // Condition blink
         );

         //
         // Process the switches' Leds
         // This is done here since we process modbus incomming data
         //
         modbus::console_leds.door    = iomux::virtual_leds.door;
         modbus::console_leds.cool    = iomux::virtual_leds.cool;
         modbus::console_leds.dust    = iomux::virtual_leds.dust;
         modbus::console_leds.release = iomux::virtual_leds.release;

         // Next stage
         reactor::yield(2);
         break;

      case 2:
      default:
         //
         // Process the push buttons LEDs
         //

         // Drive the virtual LED for ES
         iomux::led::set(
            iomux::led::Id::virtual_es,
            iomux::inputs.es ?iomux::led::Status::blinks : iomux::led::Status::off
         );

         // Override if the system is in STOP mode
         if ( iomux::inputs.es ) {
            if ( iomux::led::get(iomux::led::Id::virtual_es) ) {
               modbus::console_leds.all |= modbus::MASK_OF_PUSH_BUTTONS_LEDS;
            } else {
               modbus::console_leds.all ^= (~modbus::MASK_OF_PUSH_BUTTONS_LEDS);
            }
         } else {
            // For start/stop we use the tower light only
            modbus::console_leds.start = iomux::inputs.tower_green;
            modbus::console_leds.stop  = iomux::inputs.tower_red;

            // For the park we use the key or the tower lights
            modbus::console_leds.park =  
               iomux::inputs.tower_yellow || modbus::key == modbus::Key::Homing;

            // For the chuck, turn on when Masso controls the pneumatic
            modbus::console_leds.change_tool = iomux::inputs.chuck_pressure;

            // For the Goto), the key turn it on
            modbus::console_leds.goto0 = (modbus::key == modbus::Key::Goto0);

            // TODO : For now the key drives it
            // For the door - use the state machine output. Blinks when the
            // door is moving.
            modbus::console_leds.door = (modbus::key == modbus::Key::Door);
         }

         break;
      }
   }

   void init() {
      using namespace std::chrono;

      Pin(ISO_OUT_ES                ).init(dir_t::out);
      Pin(ISO_OUT_TOWER_LIGHT_RED   ).init(dir_t::out);
      Pin(ISO_OUT_TOWER_LIGHT_YELLOW).init(dir_t::out);
      Pin(ISO_OUT_TOWER_LIGHT_GREEN ).init(dir_t::out);
      Pin(ISO_OUT_RELEASE_STEPPER   ).init(dir_t::out);
      Pin(ISO_OUT_LASER_CROSS       ).init(dir_t::out);
      Pin(ISO_OUT_CAMERA_LIGHT      ).init(dir_t::out);

      // Use the AVR event and logic cell to drive the Modbus (RS485) LEDS automatically (0 software)
      // using the remaining timerB

      // Start the i2c mux
      TRACE_MILE(PATCH, "Starting iomux");
      iomux::init( reactor::bind(on_patch) );

      // Start the modbus
      TRACE_MILE(PATCH, "Starting modbus");
      modbus::init( reactor::bind(on_modbus_console_reply) );
   }
}  // namespace patch