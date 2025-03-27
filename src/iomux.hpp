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
         END_OF_LEDS,
         virtual_door,
         virtual_release,
         virtual_dust,
         virtual_cool,
         virtual_es,
         END_OF_VIRTUAL
      };

      constexpr auto COUNT = static_cast<uint8_t>(Id::END_OF_LEDS);
      constexpr auto VIRTUAL_FIRST_INDEX = COUNT + 1;
      constexpr auto COUNT_VIRTUAL = 
         static_cast<uint8_t>(Id::END_OF_VIRTUAL) - VIRTUAL_FIRST_INDEX;
   
      static constexpr uint16_t masks[COUNT] = {
         1<<0,  // tower_red
         1<<1,  // tower_yellow
         1<<2,  // tower_green
         1<<3,  // laser_cross
         1<<4,  // cam_light
         1<<5,  // release_steppers
         1<<6,  // console
         1<<7,  // reslay
         1<<8,  // pneumatic_hub
         1<<9,  // low_pressure
         1<<10, // chuck
         1<<11, // clean
         1<<12, // door_opening
         1<<13  // door_closing
      };

      union Virtual {
         uint8_t all;
         struct {
            uint8_t door : 1;
            uint8_t release : 1;
            uint8_t dust : 1;
            uint8_t cool : 1;
         };
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
   inline auto virtual_leds = led::Virtual{0};

   // Unique function to init the iomux and take case of the i2c
   void init(asx::reactor::Handle);

} // End of mux namespace
