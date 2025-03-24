#pragma once

#include <stdint.h>

namespace iomux {
   /// @brief Callback for processing the update inputs and outputs
   using ProcessInputCb = void(*)();

   // Define a union to combine uint16_t with bitfields
   union Led {
      uint16_t all;

      // First declared is bit0 = D5 (top LED)
      struct {
         // OC Monitoring
         uint16_t tower_red : 1;        // D5
         uint16_t tower_yellow : 1;     // D6
         uint16_t tower_green : 1;      // D7
         uint16_t laser_cross : 1;      // D8
         uint16_t cam_light : 1;        // D9
         uint16_t release_steppers : 1; // D10

         // Modbus communication statuses
         uint16_t console : 1;          // D11
         uint16_t relay : 1;            // D12
         uint16_t pneumatic_hub : 1;    // D13

         // Pneumatic monitoring
         uint16_t low_pressure : 1;     // D14
         uint16_t chuck : 1;            // D15
         uint16_t clean : 1;            // D16

         // Door control
         uint16_t door_opening : 1;     // D17
         uint16_t door_closing : 1;     // D18
      } bits;
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
         uint16_t tower_red         : 1;
         uint16_t tower_yellow      : 1;
         uint16_t tower_green       : 1;
         uint16_t door_open_close   : 1;
         uint16_t toolset_clean     : 1;
         uint16_t chuck_pressure    : 1;
         uint16_t touch_screen_beep : 1;
         uint16_t sounder           : 1;
         uint16_t laser_crossair    : 1;
         uint16_t camera_light      : 1;
         uint16_t spare_input_1     : 1;
         uint16_t spare_input_2     : 1;
         uint16_t es                : 1;
         uint16_t spindle_is_on     : 1;
         uint16_t door_is_down      : 1;
         uint16_t door_is_up        : 1;
      };
   };

   //
   // Inline instances for direct access
   //
   inline auto inputs = Inputs{0};

   inline auto outputs = Outputs{0};

   inline auto leds = Led{0};

   // Unique function to init the iomux and take case of the i2c
   void init(ProcessInputCb cb);

} // End of mux namespace
