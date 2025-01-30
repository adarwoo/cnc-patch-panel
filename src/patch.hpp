#pragma once

#include <array>
#include <asx/ioport.hpp>
#include <cstdint>

#include "conf_board.h"

namespace patch {
   // Define a union to combine uint16_t with bitfields
   union Relays {
      uint16_t all;
      struct {
         uint16_t dust : 1;
         uint16_t cool : 1;
         uint16_t spare : 1;
      } bits;
   };

   union IsolatedOutputs {
      uint16_t all;
      struct {
         uint16_t release : 1;
         uint16_t spare : 2;
      } bits;
   };

   union Switches {
      uint16_t all;
      struct {
         uint16_t cool : 1;
         uint16_t dust : 1;
         uint16_t release : 1;
         uint16_t door : 1;
      } bits;
   };

   // Define a union to combine uint16_t with bitfields
   union PneumaticCoils {
      uint16_t all;
      struct {
         uint16_t air_blast : 1;
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
         uint16_t start : 1;
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
         uint16_t mso_in0 : 1;  // Key pressed
         uint16_t mso_in1 : 1;  // Relay ? 1-2
         uint16_t mso_in2 : 1;  // door state machine ?
         uint16_t mso_in3 : 1;
         uint16_t mso_in4 : 1;
         uint16_t mso_in5 : 1;
         uint16_t mso_in6 : 1;
         uint16_t mso_in7 : 1;
         uint16_t mso_in8 : 1;
         uint16_t mso_in9 : 1;
         uint16_t mso_in10 : 1;
         uint16_t mso_in11 : 1;
         uint16_t spindle_on : 1;
         uint16_t door_up : 1;
         uint16_t door_down : 1;
         uint16_t error : 1;
      } bits;
   };

   struct OutputToMasso : public asx::ioport::Pin {
      OutputToMasso(asx::ioport::PinDef _pin) : asx::ioport::Pin(_pin) {
         init(asx::ioport::dir_t::out);
      }
   };

   enum MassoInputPins : uint8_t {
      start_key,
      stop_key,
      home_key,
      goto0_key,
      park_key,
      tool_change_key,
      door_key,
      fn1_key,
      fn2_key,
      fn3_key,
      fn4_key,
      fn5_key,
      fn6_key
   };

   std::array<OutputToMasso, 13> outputs_to_masso = {
      asx::ioport::PinDef{MSO_OUT0},
      asx::ioport::PinDef{MSO_OUT1},
      asx::ioport::PinDef{MSO_OUT2},
      asx::ioport::PinDef{MSO_OUT3},
      asx::ioport::PinDef{MSO_OUT4},
      asx::ioport::PinDef{MSO_OUT5},
      asx::ioport::PinDef{MSO_OUT6},
      asx::ioport::PinDef{MSO_OUT7},
      asx::ioport::PinDef{MSO_OUT8},
      asx::ioport::PinDef{MSO_OUT9},
      asx::ioport::PinDef{MSO_OUT10},
      asx::ioport::PinDef{MSO_OUT11},
      asx::ioport::PinDef{MSO_OUT12}
   };

   struct start {};
   struct i2c_ready {};
   struct polling {};
}  // namespace patch
