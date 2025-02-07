#pragma once

#include <array>
#include <asx/ioport.hpp>
#include <cstdint>

#include "conf_board.h"

#ifndef DOMAIN_PATCH_ENABLED
#  define DOMAIN_PATCH_ENABLED 0
#endif

namespace patch {
   // Define a union to combine uint16_t with bitfields
   union Relays {
      uint16_t all;
      struct {
         uint16_t dust : 1; // Bit0
         uint16_t cool : 1;
         uint16_t spare : 1;
      } bits;
   };

   /// @brief Isolated output control values
   union IsolatedOutputs {
      uint16_t all;
      struct {
         uint16_t release : 1; // Bit0
         uint16_t spare : 2;
      } bits;
   };

   union Switches {
      uint16_t all;
      struct {
         uint16_t cool : 1; // Bit0
         uint16_t dust : 1;
         uint16_t release : 1;
         uint16_t door : 1;
      } bits;
   };

   // Define a union to combine uint16_t with bitfields
   union PneumaticCoils {
      uint16_t all;
      struct {
         uint16_t air_blast : 1; // Bit0
         uint16_t chuck : 1;
         uint16_t spindle_clean : 1;
         uint16_t door_pull : 1;
         uint16_t chuck_open : 1;
         uint16_t door_push : 1;
      } bits;
   };

   // Define a union to combine uint16_t with bitfields
   union ConsoleLeds {
      uint16_t all;
      struct {
         uint16_t start : 1;  // Bit0
         uint16_t stop : 1;
         uint16_t home : 1;
         uint16_t goto0 : 1;
         uint16_t park : 1;
         uint16_t change_tool : 1;
      } bits;
   };

   // Define a union to combine uint16_t with bitfields
   union LedFB {
      uint16_t all;
      struct {
         // Console key presses (LED1 to LED4)
         uint16_t console_key_code : 3; // LED1 to LED3 (3 bits)
         uint16_t console_shift : 1;    // LED4 (1 bit)

         // Controls (LED5 to LED8)
         uint16_t override_door : 1; // LED5 (1 bit)
         uint16_t dust : 1;          // LED6 (1 bit)
         uint16_t cooling : 1;       // LED7 (1 bit)
         uint16_t free_axis : 1;     // LED8 (1 bit)

         // Inputs (LED9 to LED12)
         uint16_t spare_input : 1;   // LED9 (1 bit)
         uint16_t spindle_on : 1;    // LED10 (1 bit)
         uint16_t door_is_opened : 1; // LED11 (1 bit)
         uint16_t door_is_closed : 1; // LED12 (1 bit)

         // Errors (LED13 to LED16)
         uint16_t general_error : 1; // LED13 (1 bit)
         uint16_t modbus_error : 1;  // LED14 (1 bit)
         uint16_t door_opening : 1;  // LED15 (1 bit)
         uint16_t door_closing : 1;  // LED16 (1 bit)
      } bits;
   };

   /// @brief patch_out_to_masso_in
   struct MassoInputPins {
      uint16_t button_start           : 1;
      uint16_t button_home            : 1;
      uint16_t button_stop            : 1;
      uint16_t button_go_to_home      : 1;
      uint16_t button_go_to_parking   : 1;
      uint16_t chuck_clamp_unclamp    : 1;
      uint16_t door_open_close        : 1;
      uint16_t autoload_g_code        : 1;
      uint16_t door_sensor_input      : 1;
      uint16_t probe                  : 1;
      uint16_t tool_setter            : 1;
      uint16_t air_pressure_low_alarm : 1;
      uint16_t reserved : 4;
   };

   /// @brief Mask and direction value for the IO expander used for I/O
   static constexpr auto IO_MASK = uint16_t{0b1111};

   struct start {};
   struct i2c_ready {};
   struct polling {};

   void init();
}  // namespace patch
