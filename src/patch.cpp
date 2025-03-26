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

   // Map comm status to LED status
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

   // Called once per i2c cycle
   // This cycle is much faster that the modbus cycle, so it can be used to sync all
   void on_refresh() {
      // Sync pneumatics
      iomux::outputs.air_pressure_low_alarm = modbus::pressure_in;
      modbus::coils.chuck = iomux::inputs.chuck_pressure;

      //
      // LEDS
      //

      // Air LEDs
      iomux::led::set(iomux::led::Id::clean,         get_clean_led_status());
      iomux::led::set(iomux::led::Id::low_pressure,  modbus::pressure_in);
      iomux::led::set(iomux::led::Id::chuck,         iomux::inputs.chuck_pressure);

      // Modbus comms LEDs
      iomux::led::set(iomux::led::Id::console,       to_led_status(modbus::console_comms_status));
      iomux::led::set(iomux::led::Id::pneumatic_hub, to_led_status(modbus::pneu_comms_status));
      iomux::led::set(iomux::led::Id::relay,         to_led_status(modbus::relay_comms_status));

      // OC Outputs
      iomux::led::set(iomux::led::Id::cam_light,     iomux::inputs.camera_light);
      iomux::led::set(iomux::led::Id::laser_cross,   iomux::inputs.laser_crossair);
      iomux::led::set(iomux::led::Id::cam_light,     iomux::inputs.camera_light);
      iomux::led::set(iomux::led::Id::tower_green,   iomux::inputs.tower_green);
      iomux::led::set(iomux::led::Id::tower_red,     iomux::inputs.tower_red);
      iomux::led::set(iomux::led::Id::tower_yellow,  iomux::inputs.tower_yellow);

      // Door
      iomux::led::set(iomux::led::Id::door_closing,  false); // TODO
      iomux::led::set(iomux::led::Id::door_opening,  false); // TODO

      //
      // Drive MPU GPIOs
      //
      Pin(ISO_OUT_ES                ).set( iomux::inputs.es );
      Pin(ISO_OUT_TOWER_LIGHT_RED   ).set( iomux::inputs.tower_red );
      Pin(ISO_OUT_TOWER_LIGHT_YELLOW).set( iomux::inputs.tower_yellow );
      Pin(ISO_OUT_TOWER_LIGHT_GREEN ).set( iomux::inputs.tower_green );
      Pin(ISO_OUT_LASER_CROSS       ).set( iomux::inputs.laser_crossair );
      Pin(ISO_OUT_CAMERA_LIGHT      ).set( iomux::inputs.camera_light );

      //
      // Drive Masso inputs (our outputs) which are not keys
      //
      iomux::outputs.door_sensor_input = modbus::switches.door || iomux::inputs.door_is_down;
   }

   void on_modbus_console_reply() {
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
      Pin(ISO_OUT_RELEASE_STEPPER).set( modbus::switches.release );
      modbus::relays.cool = modbus::switches.cool;
      modbus::relays.dust = modbus::switches.dust;

      //
      // Map lights
      //

      // Switches
      modbus::console_leds.door = 
      modbus::console_leds.cool = 
      modbus::console_leds.dust = 
      modbus::console_leds.release = iomux::led::get( modbus::switches.release );
   }

   void init() {
      using namespace std::chrono;

      Pin(ISO_OUT_ES                ).init(dir_t::out);
      Pin(ISO_OUT_TOWER_LIGHT_RED   ).init(dir_t::out);
      Pin(ISO_OUT_TOWER_LIGHT_YELLOW).init(dir_t::out);
      Pin(ISO_OUT_TOWER_LIGHT_GREEN ).init(dir_t::out);
      Pin(ISO_OUT_RELEASE_STEPPER   ).init(dir_t::out);
      Pin(ISO_OUT_LASER_CROSS       ).init(dir_t::out);
      Pin(ISO_OUT_CAMERA_LIGHT).init(dir_t::out);

      // Start the i2c mux
      iomux::init( reactor::bind(on_refresh) );

      // Start the modbus
      modbus::init( reactor::bind(on_modbus_console_reply) );
   }
}  // namespace patch