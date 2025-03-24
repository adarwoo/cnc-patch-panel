#include <chrono>
#include <array>

#include <asx/reactor.hpp>
#include <asx/timer.hpp>
#include <asx/pca9555.hpp>

#include "iomux.hpp"

using namespace asx;
using std::chrono;

constexpr auto BLINK_PERIOD = 500ms;

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
   static auto react_on_input_change  = reactor::null;

   // Forward declaration
   static void on_refresh();

   // Reactor to call periodically
   auto react_on_refresh = reactor::bind(on_refresh);

   static auto iomux_out = i2c::PCA9555(0);        // IOMux 0 For input pins
   static auto iomux_in  = i2c::PCA9555(6);        // IOMux 6 For input pins
   static auto iomux_led = i2c::PCA9555(1);        // IOMux 1 For LEDs

   // Compute the delta
   auto prev_inputs = Inputs{0};

   // Manage blinking for all LEDs
   auto led_blink_next_change = std::array<Led::Id::COUNTOF, timer::time_point>{};
   
   // Value of the LED output
   auto leds_fb = uint16_t{0};

   void on_i2c_operation(i2c::status_code_t code) {
      alert_and_stop_if(code != i2c::status_code_t::STATUS_OK);

      switch (stage) {
         case InitStage::init_led_val: iomux_led.set_value(0xAAAA, on_i2c_operation); break;
         case InitStage::init_led_dir: iomux_led.set_dir(0, on_i2c_operation); break;
         case InitStage::init_out_val: iomux_out.set_value(0, on_i2c_operation); break;
         case InitStage::init_out_dir: iomux_out.set_value(0, on_i2c_operation); break;
         case InitStage::init_in_dir:  iomux_in.set_dir(0xffff, on_i2c_operation); break;
         case InitStage::init_poll:
            using namespace std::chrono;
            react_on_refresh.repeat(1000ms);
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
            Inputs delta = prev_inputs.all ^ inputs.all;
            if ( delta ) {
               react_on_input_change(delta);
            }
            return;
         default:
            alert_and_stop_if(true);
            break;
      }

      ++stage;
   }

   void on_refresh() {
      auto status = i2c::Master::get_status();

      if ( status == i2c::status_code_t::STATUS_OK ) {
         stage = InitStage::read_input;

         // Update blinking
         auto now = timer::now();

         for ( led::Id id=0; id<led::Id::COUNTOF; ++id) {
            auto next_change = led_blink_next_change[id];

            if ( next_change > now ) {
               leds_fb ^= mask_of(id);
               led_blink_next_change[id] = next_change + BLINK_PERIOD;
            }
         }

         // Update
         on_i2c_operation(status);
      }
   }

   namespace led {
      Status state_of(Id id) {
         if ( led_blink_next_change[id] != timer::time_point{0} ) {
            return Status::blinks;
         }

         if ( leds_fb & mask_of(id) ) {
            return Status::on;
         }

         return Status::off;
      }

      void turn_on(Id id) {
         led_blink_next_change[id] = timer::time_point{0};
         leds_fb |= mask_of(id);
      }

      void turn_off(Id id) {
         led_blink_next_change[id] = timer::time_point{0};
         leds_fb &= ~mask_of(id);
      }

      void blink(Id id) {
         if ( led_blink_next_change[id] == timer::time_point{0} ) {
            led_blink_next_change[id] = timer::now() + BLINK_PERIOD;
         }

         leds_fb |= mask_of(id);
      }

      void set(Id id, Status status) {
         switch (status) {
         case on: turn_on(id); break
         case off: turn_off(id); break;
         case blink: blink(id); break;
         default:
            break;
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



