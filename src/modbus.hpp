#pragma once

#include <cstdint>

namespace modbus {
   /// @brief Callback for processing the update inputs and outputs
   using ProcessInputCb = void(*)();

   /// @brief Status of a modbus communication
   struct ModbusComm {
      enum class ModbusCommStatus : int8_t {
         error = -2,
         down = -1,
         ok = 0,
         starting = 1
      } status;

      // Error count
      uint16_t error_count;
      uint16_t nocomm_count;
      uint16_t good_comm_count;

      ///< Update communication stats
      void report_nocomm();
      void report_success();
      void report_error();

      ///< Construt
      ModbusComm() : status{0}, error_count{0}, nocomm_count{0}, good_comm_count{0} {
      }
   };

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
         uint16_t cool : 1; // Bit0
         uint16_t dust : 1;
         uint16_t release : 1;
         uint16_t door : 1;
      } bits;
   };

   static inline auto relays = Relays{0};
   static inline auto switches = Switches{0};
   static inline auto coils = PneumaticCoils{0};
   static inline auto console_leds = ConsoleLeds{0};
   static inline auto relay_comms_status = ModbusComm{};
   static inline auto pneu_comms_status = ModbusComm{};
   static inline auto console_comms_status = ModbusComm{};

   void init(ProcessInputCb);
}  // namespace modbus
