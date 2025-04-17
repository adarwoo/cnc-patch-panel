/*
 * Console modbus device main entry point.
 */
#include <chrono>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <trace.h>
#include <debug.h>
#include <asx/ioport.hpp>
#include <asx/reactor.hpp>

#include "patch.hpp"
#include "door.hpp"
#include "conf_board.h"

using namespace asx::ioport;
using namespace std::chrono;

/**
 * Combine the event system with the configurable custom logic to drive
 * the UART LEDs without software.
 * Upon detecting activity, a pulse is generated so the activity can be seen clearly.
 * Both Timers B are used. TCB0 is used for Rx and TCB1 for Tx
 * The clock input of the timers is wired from the PIT timers through an event channel.
 *
 * Rx Pin on PA5 is connected to TCB0 0.WO
 * Tx Pin on PA7 is connected to LUT1.OUT
 */
void setup_modbus_activity_leds() {
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"

   // Pulse duration

   // Define a custom duration type representing one tick of the PIT/64 clock
   using tick_duration = duration<int64_t, std::ratio<1, 32768 / 64>>;   
   
   // Set the pulse duration
   constexpr auto pulse_duration = duration_cast<tick_duration>(5ms);

   // Event channels configuration
   EVSYS.CHANNEL0 = EVSYS_CHANNEL0_PORTA_PIN1_gc;    // Rx/Tx activity
   EVSYS.CHANNEL1 = EVSYS_CHANNEL1_PORTA_PIN4_gc;    // XDIR direction selection
   EVSYS.CHANNEL2 = EVSYS_CHANNEL2_CCL_LUT0_gc;      // Output of the LUT2 (RTX & ~XDIR)
   EVSYS.CHANNEL3 = EVSYS_CHANNEL3_RTC_PIT_DIV64_gc; // Output of the periodic timer

   EVSYS.USERCCLLUT0A  = EVSYS_USER_CHANNEL0_gc;     // LUT2-EVENTA  = Ch0 [Rx/Tx activity]
   EVSYS.USERCCLLUT0B  = EVSYS_USER_CHANNEL1_gc;     // LUT2-EVENTB  = Ch1 [XDIR]
  
   EVSYS.USERTCB0CAPT  = EVSYS_USER_CHANNEL2_gc;     // TCB0 Capture = Ch2 [LUT2-OUT=RxTx & ~XDIR]
   EVSYS.USERTCB0COUNT = EVSYS_USER_CHANNEL3_gc;     // TCB0 count uses channel 3
   EVSYS.USERTCB1CAPT  = EVSYS_USER_CHANNEL1_gc;     // TCB1 Capture = Ch1 [XDIR]
   EVSYS.USERTCB1COUNT = EVSYS_USER_CHANNEL3_gc;     // TCB1 count uses channel 3
   
   // LUT0 configurations : IN0[A]=Ch0/RTX | IN1[B]=Ch1/XDIR | IN2[-] => Channel 2
   CCL.LUT0CTRLB = CCL_INSEL0_EVENTA_gc | CCL_INSEL1_EVENTB_gc;
   CCL.LUT0CTRLC = 0;
   CCL.TRUTH0    = 1; // LUT2_OUT = (~A & ~B) => CH0 & ~CH1 => ~RTX & ~DIR
   CCL.LUT0CTRLA = CCL_ENABLE_bm;

   // TCB0 configuration : Drives the Rx pin directly
   TCB0.CCMP = pulse_duration.count();
   TCB0.CNT = pulse_duration.count();
   TCB0.EVCTRL = TCB_CAPTEI_bm | TCB_FILTER_bm; // Turn on event detection
   TCB0.CTRLB = TCB_ASYNC_bm | TCB_CCMPEN_bm | TCB_CNTMODE_SINGLE_gc; // Enable the output
   TCB0.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm; // Use the event channel as a clock source

   // TCB1 -> Drives the Tx pin directly
   TCB1.CCMP = pulse_duration.count();
   TCB1.CNT = pulse_duration.count();
   TCB1.EVCTRL = TCB_CAPTEI_bm | TCB_FILTER_bm; // Turn on event detection
   TCB1.CTRLB = TCB_ASYNC_bm | TCB_CCMPEN_bm | TCB_CNTMODE_SINGLE_gc; // Enable the output
   TCB1.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm;  // Use the event channel as a clock source

   // Activate the CCL for both
   CCL.CTRLA = CCL_ENABLE_bm;
}


int main()
{
   TRACE_MILE(APP, "CNC Patch starting");

   // Init our unique debug pin called TRACE
   Pin(DEBUG_TRACE).init(dir_t::out, value_t::low);

   // Initialse the patch management (i2c, gpio and modbus sequencer)
   patch::init();

   // Initialse the door controller
   // TODO door::init();

   // Turn on all LEDs for 2 seconds
   Pin(LED_MODBUS_RX).init(dir_t::out, value_t::high);
   Pin(LED_MODBUS_TX).init(dir_t::out, value_t::high);
   Pin(ALERT_OUTPUT_PIN).set(true);

   // Use LEDs as intended past 2s
   asx::reactor::bind([]() {
      Pin(ALERT_OUTPUT_PIN).clear();
      setup_modbus_activity_leds();
   }).delay(2s);

   // Run the reactor/scheduler
   asx::reactor::run();
}
