#include <chrono>
#include <asx/reactor.hpp>
#include <asx/pca9555.hpp>

#include "iomux.hpp"

using namespace asx;

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
   static ProcessInputCb process_cb = nullptr;

   // Forward declaration
   static void on_refresh();

   // Reactor to call periodically
   auto react_on_refresh = reactor::bind(on_refresh);

   static auto iomux_out = i2c::PCA9555(0);        // IOMux 0 For input pins
   static auto iomux_in  = i2c::PCA9555(6);        // IOMux 6 For input pins
   static auto iomux_led = i2c::PCA9555(1);        // IOMux 1 For LEDs

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
            iomux_led.set_value(leds.all, on_i2c_operation);
            break;
         case InitStage::update_output:
            iomux_out.set_value(outputs.all, on_i2c_operation);
            break;
         case InitStage::ready:
            process_cb();
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
         on_i2c_operation(status);
      }
   }

   void init(ProcessInputCb cb) {
      using namespace asx::i2c;

      process_cb = cb;
      i2c::Master::init( 400_KHz );

      on_i2c_operation( i2c::Master::get_status() );
   }

} // namespace mux



