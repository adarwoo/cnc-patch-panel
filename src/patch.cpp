#include <chrono>
#include <asx/ioport.hpp>
#include <asx/reactor.hpp>

#include "iomux.hpp"
#include "patch.hpp"

namespace patch {
   using namespace asx::ioport;
   using namespace asx;

   void led_test_over() {
      iomux::leds.all = 1;
   }

   void on_process_masso_outputs() {
      iomux::leds.all <<= 1;

      if ( iomux::leds.all == 0 ) {
         iomux::leds.all = 1;
      }
   }

   void init() {
      iomux::leds.all = 0xffff; // Turn all LEDs on on boot
      iomux::init(on_process_masso_outputs);

      // Turn off led after 2 seconds
      using namespace std::chrono;
      reactor::bind(led_test_over).delay(2s);
   }

}  // namespace patch