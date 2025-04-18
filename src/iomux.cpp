#include <chrono>
#include <array>

#include <asx/reactor.hpp>
#include <asx/timer.hpp>
#include <asx/pca9555.hpp>

#include "iomux.hpp"

using namespace asx;
using namespace std::chrono;

/// @brief Period between blinking LED state change
constexpr auto BLINK_PERIOD = 250ms;

// Split the LED refresh into iterations
constexpr auto MAX_LEDS_TO_PROCESS_PER_ITERATIONS = 2;

namespace iomux
{
   static enum class InitStage : uint8_t {
      init_led_val,
      init_led_dir,
      init_out_val,
      init_out_dir,
      init_in_dir,
      init_poll,
      read_input,
      update_led,
      update_output,
      ready
   } stage = InitStage::init_led_val;

   // Prefix increment helper
   InitStage& operator++(InitStage& x) {
      x = static_cast<InitStage>(static_cast<uint8_t>(x) + 1);
      return x;
   }

   // Callback to invoke when the input is updated
   static auto react_on_input_change  = reactor::Handle{};

   // Forward declaration
   static void on_refresh(uint8_t yield_to);

   // Reactor to call periodically
   auto react_on_refresh = reactor::bind(on_refresh);

   static auto iomux_out = i2c::PCA9555(0);        // IOMux 0 For input pins
   static auto iomux_in  = i2c::PCA9555(6);        // IOMux 6 For input pins
   static auto iomux_led = i2c::PCA9555(1);        // IOMux 1 For LEDs

   // Compute the delta
   auto prev_inputs = Inputs{0};

   // Manage blinking for all LEDs
   auto led_blink_next_change =
      std::array<timer::steady_clock::time_point, led::COUNT>{};

   // Value of the LED output
   auto leds_fb = uint16_t{0};

   void on_i2c_operation(i2c::status_code_t code) {
      alert_and_stop_if(code != i2c::status_code_t::STATUS_OK);

      switch (stage) {
         case InitStage::init_led_val: iomux_led.set_value(0xffff, on_i2c_operation); break;
         case InitStage::init_led_dir: iomux_led.set_dir  (0,      on_i2c_operation); break;
         case InitStage::init_out_val: iomux_out.set_value(0,      on_i2c_operation); break;
         case InitStage::init_out_dir: iomux_out.set_value(0,      on_i2c_operation); break;
         case InitStage::init_in_dir:  iomux_in. set_dir  (0xffff, on_i2c_operation); break;
         case InitStage::init_poll:
            using namespace std::chrono;
            react_on_refresh.repeat<uint8_t>(1s, 10ms, 0);
            break;
         case InitStage::read_input:
            iomux_in.read(on_i2c_operation);
            break;
         case InitStage::update_led:
            inputs.all = iomux_in.get_value<uint16_t>();
            iomux_led.set_value(leds_fb, on_i2c_operation);
            break;
         case InitStage::update_output:
            iomux_out.set_value(outputs.all, on_i2c_operation);
            break;
         case InitStage::ready:
            react_on_input_change(0);
            return;
         default:
            alert_and_stop_if(true);
            break;
      }

      ++stage;
   }

   namespace led {
      constexpr auto time_zero =
         timer::steady_clock::time_point(timer::steady_clock::duration::zero());

      static inline uint16_t mask_of(const Id id) {
         return masks[static_cast<uint8_t>(id)];
      }

      Status state_of(Id id) {
         auto _id = static_cast<uint8_t>(id);

         if ( led_blink_next_change[_id] != time_zero ) {
            return Status::blinks;
         }

         if ( leds_fb & mask_of(id) ) {
            return Status::on;
         }

         return Status::off;
      }

      // Return the status of the LED (on or off) at the time of calling
      bool get(Id id) {
         return leds_fb & mask_of(id);
      }

      void set(Id id, bool onoff) {
         auto _id = static_cast<uint8_t>(id);

         led_blink_next_change[_id] = time_zero;

         if ( onoff ) {
            leds_fb |= mask_of(id);
         } else {
            leds_fb &= ~mask_of(id);
         }
      }

      void blink(Id id) {
         auto _id = static_cast<uint8_t>(id);

         if ( led_blink_next_change[_id] == time_zero ) {
            led_blink_next_change[_id] = timer::steady_clock::now();
            leds_fb |= mask_of(id);
         }
      }

      void set(Id id, Status status) {
         switch (status) {
         case Status::on:     set(id, true); break;
         case Status::off:    set(id, false); break;
         case Status::blinks: blink(id); break;
         default:
            break;
         }
      }
   }

   /**
    * Entry point for the periodic refresh of the i2c ops
    */
   void on_refresh(uint8_t yield_to) {
      static auto now = timer::steady_clock::now();
      auto status = i2c::Master::get_status();

      if ( status == i2c::status_code_t::STATUS_OK ) {
         switch (yield_to) {
         case 0:
         {
            // Force the state machine to go back to read
            stage = InitStage::read_input;

            // Update blinking
            now = timer::steady_clock::now();

            // Yield till next time
            reactor::yield(1);

            break;
         }
         default:
         {
            // Go through all the regular LEDs to handle blinking
            uint8_t id = (yield_to - 1) * MAX_LEDS_TO_PROCESS_PER_ITERATIONS;
            uint8_t id_to = id + MAX_LEDS_TO_PROCESS_PER_ITERATIONS;
            bool last = (id_to >= led_blink_next_change.size());

            if ( last ) {
               id_to = led_blink_next_change.size();
            }

            while (id < id_to) {
               auto next_change = led_blink_next_change[id];

               if ( next_change >= now ) {
                  leds_fb ^= led::masks[id];
                  led_blink_next_change[id] = next_change + BLINK_PERIOD;
               }

               ++id;
            }

            if ( last ) {
               // Update
               on_i2c_operation(status);
            } else {
               reactor::yield(yield_to + 1);
            }

            break;
         }
         }
      }
   }

   void init(reactor::Handle on_change) {
      using namespace asx::i2c;

      react_on_input_change = on_change;

      i2c::Master::init( 400_KHz );

      on_i2c_operation( i2c::Master::get_status() );
   }

} // namespace mux
