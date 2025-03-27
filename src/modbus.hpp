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
      starting = 1
   };

   // Define a union to combine uint16_t with bitfields
   union Relays {
      uint16_t all;
      struct {
         uint16_t dust : 1; // Bit0
         uint16_t cool : 1;
         uint16_t spare : 1;
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
         uint16_t start : 1;  // Bit0
         uint16_t stop : 1;
         uint16_t home : 1;
         uint16_t goto0 : 1;
         uint16_t park : 1;
         uint16_t change_tool : 1;
         uint16_t cool : 1; // Bit0
         uint16_t dust : 1;
         uint16_t release : 1;
         uint16_t door : 1;
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

   static inline auto relays                 = Relays{0};
   static inline auto switches               = Switches{0};
   static inline auto key                    = Key{Key::None};
   static inline auto coils                  = PneumaticCoils{0};
   static inline auto console_leds           = ConsoleLeds{0};
   static inline auto relay_comms_status     = CommStatus{};
   static inline auto pneu_comms_status      = CommStatus{};
   static inline auto console_comms_status   = CommStatus{};
   static inline auto pressure_in            = bool{false};

   void init(asx::reactor::Handle);
}  // namespace modbus
