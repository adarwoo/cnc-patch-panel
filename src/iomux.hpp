#pragma once

#include <cstdint>
#include <asx/reactor.hpp>
#include <asx/timer.hpp>

namespace iomux {
   /// @brief Callback for processing the update inputs and outputs
   using ProcessInputCb = void(*)();

   namespace led {
      enum class Id : uint8_t {
         tower_red = 0,
         tower_yellow,
         tower_green,
         laser_cross,
         cam_light,
         release_steppers,
         console,
         relay,
         pneumatic_hub,
         low_pressure,
         chuck,
         clean,
         door_opening,
         door_closing,
         virtual_es,
         END_OF_LEDS
      };

      constexpr auto COUNT = static_cast<uint8_t>(Id::END_OF_LEDS);

      constexpr uint16_t BM(auto port, auto bit_pos) {
         uint8_t shift_by = bit_pos;

         // The port 0 is sent first (as per the datasheet) so Port0 is the MSB
         if ( port == 1 ) {
            shift_by += 8;
         }

         return 1 << shift_by;
      }

      static constexpr uint16_t masks[COUNT] = {
         BM(0,0),  // tower_red
         BM(0,1),  // tower_yellow
         BM(0,2),  // tower_green
         BM(0,3),  // laser_cross
         BM(0,4),  // cam_light
         BM(0,5),  // release_steppers
         BM(0,6),  // console comm status
         BM(1,0),  // relay comm status
         BM(1,1),  // pneumatic_hub comm status
         BM(1,2),  // low_pressure
         BM(1,3),  // chuck
         BM(1,4),  // clean
         BM(1,5),  // door_opening
         BM(1,6),  // door_closing
         BM(1,7)   // Virtual ES (Not connected)
      };

      enum class Status : uint8_t {
         off,
         on,
         blinks
      };

      Status state_of(Id);
      bool get(Id);
      void blink(Id);
      void set(Id, Status);
      void set(Id id, bool onoff);
   };

   /// @brief CNC Center outputs to Masso inputs
   union Outputs {
      uint16_t all;

      struct {
         uint16_t button_start           : 1;
         uint16_t button_home            : 1;
         uint16_t button_stop            : 1;
         uint16_t button_go_to_home      : 1;
         uint16_t button_go_to_parking   : 1;
         uint16_t chuck_clamp_unclamp    : 1;
         uint16_t door_open_close        : 1;
         uint16_t reserved               : 4;
         uint16_t air_pressure_low_alarm : 1;
         uint16_t door_sensor_input      : 1;
         uint16_t autoload_g_code1       : 1;
         uint16_t autoload_g_code2       : 1;
         uint16_t autoload_g_code3       : 1;
      };
   };

   /// @brief CNC Center outputs to Masso inputs
   union Inputs {
      uint16_t all;

      struct {
         // IO 0 0->7
         uint16_t chuck_pressure    : 1;
         uint16_t toolset_clean     : 1;
         uint16_t door_open_close   : 1;
         uint16_t tower_green       : 1;
         uint16_t tower_yellow      : 1;
         uint16_t tower_red         : 1;
         uint16_t es                : 1;
         uint16_t spare_input_2     : 1;

         // IO 1 0->7
         uint16_t spare_input_1     : 1;
         uint16_t camera_light      : 1;
         uint16_t laser_crossair    : 1;
         uint16_t touch_screen_beep : 1;
         uint16_t sounder           : 1;
         uint16_t spindle_is_on     : 1;
         uint16_t door_is_down      : 1;
         uint16_t door_is_up        : 1;
      };
   };

   //
   // Inline instances for direct access
   //
   inline auto inputs       = Inputs{0};
   inline auto outputs      = Outputs{0};

   // Unique function to init the iomux and take case of the i2c
   void init(asx::reactor::Handle);

} // End of mux namespace
