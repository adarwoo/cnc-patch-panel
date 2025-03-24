#pragma once

#include <cstdint>

namespace modbus {
   // Define a union to combine uint16_t with bitfields
   union Relays {
      uint16_t all;
      struct {
         uint16_t dust : 1; // Bit0
         uint16_t cool : 1;
         uint16_t spare : 1;
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

   void init();
}  // namespace modbus
