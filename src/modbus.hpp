#pragma once

#include <cstdint>
#include <asx/reactor.hpp>

namespace modbus {
   /// @brief Callback for processing the update inputs and outputs
   using ProcessInputCb = void(*)();

   /// @brief Status of a modbus communication
   enum class CommStatus : int8_t {
      error = -2,
      down = -1,
      ok = 0,
   };

   // Define a union to combine uint16_t with bitfields
   union Relays {
      uint8_t all;
      struct {
         uint8_t dust : 1; // Bit0
         uint8_t cool : 1;
         uint8_t spare : 1;
      };
   };

   union Switches {
      uint16_t all;
      struct {
         uint16_t cool : 1; // Bit0
         uint16_t dust : 1;
         uint16_t release : 1;
         uint16_t door : 1;
      };
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
      };
   };

   // Define a union to combine uint16_t with bitfields
   union ConsoleLeds {
      uint16_t all;

      struct {
         uint8_t lsb;
         uint8_t msb;
      };

      struct {
         uint16_t start       : 1;  // Bit0
         uint16_t stop        : 1;
         uint16_t home        : 1;
         uint16_t goto0       : 1;
         uint16_t park        : 1;
         uint16_t change_tool : 1;
         uint16_t reserved0   : 2;
         uint16_t cool        : 1; // Bit0
         uint16_t dust        : 1;
         uint16_t release     : 1;
         uint16_t door        : 1;
         uint16_t reserved1   : 3;
         uint16_t sounder     : 1; // Not an LED! The sounder is mapped as an LED for efficient operation
      };
   };

   // Mask for the push buttons LEDs
   static constexpr auto MASK_OF_PUSH_BUTTONS_LEDS = 0b111111;

   enum class Key : uint8_t {
      None   = 0,
      Start  = 1,
      Stop   = 2,
      Homing = 3,
      Goto0  = 4,
      Park   = 5,
      Chuck  = 6,
      Door   = 7,
      P1     = 8,
      P2     = 9,
      P3     =10,
      P4     =11,
      P5     =12,
      P6     =13,
      P7     =14
   };

   //
   // To be used at will. These are
   //
   extern Relays         relays;
   extern Switches       switches;
   extern Key            key;
   extern PneumaticCoils coils;
   extern ConsoleLeds    console_leds;
   extern bool           pressure_in;

   extern CommStatus     relay_comms_status;
   extern CommStatus     pneu_comms_status;
   extern CommStatus     console_comms_status;

   static constexpr auto door_led_id = 0;
   static constexpr auto dust_led_id = 1;
   static constexpr auto cool_led_id = 2;
   static constexpr auto release_led_id = 3;

   // Change the state of an led
   void set_led(uint8_t id, bool onoff, bool override);

   // Read the state (on/off) of an LED
   bool get_led(uint8_t id);

   // Initialise passing a handler following a sucessfull reply
   void init(asx::reactor::Handle);

   // Send a 'beep' request
   void beep();
}  // namespace modbus
