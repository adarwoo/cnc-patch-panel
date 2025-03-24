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
         COUNTOF
      };
   
      static constexpr uint16_t mask_of[static_cast<uint8_t>(Id::COUNTOF)] = {
         0b1,        // tower_red
         0b10,       // tower_yellow
         0b100,      // tower_green
         0b1000,     // laser_cross
         0b10000,    // cam_light
         0b100000,   // release_steppers
         0b1000000,  // console
         0b10000000, // relay
         0b100000000, // pneumatic_hub
         0b1000000000, // low_pressure
         0b10000000000, // chuck
         0b100000000000, // clean
         0b1000000000000, // door_opening
         0b10000000000000 // door_closing
      }; 

      enum class Status : uint8_t {
         off,
         on,
         blinks
      };

      Status state_of(Id);
      void turn_on(Id);
      void turn_off(Id);
      void blink(Id);
      void set(Id, Status);
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

   // Unique function to init the iomux and take case of the i2c
   void init(reactor::Handle);

} // End of mux namespace
