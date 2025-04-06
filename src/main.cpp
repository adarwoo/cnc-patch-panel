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

   //
   // Setup common channel for Rx and Tx
   //

   // Channel generators
   EVSYS_CHANNEL0 = EVSYS_CHANNEL0_PORTA_PIN1_gc; // Rx/Tx activity
   EVSYS_CHANNEL1 = EVSYS_CHANNEL1_PORTA_PIN4_gc; // XDIR direction selection
   EVSYS_CHANNEL2 = EVSYS_CHANNEL2_CCL_LUT0_gc;   // Output of the LUT0 (RTX & ~XDIR)
   EVSYS_CHANNEL3 = EVSYS_CHANNEL3_CCL_LUT2_gc;   // Output of the LUT2 (RTX & XDIR)
   EVSYS_CHANNEL4 = EVSYS_CHANNEL4_TCB1_CAPT_gc;  // Output of TCB1 capture
   EVSYS_CHANNEL5 = EVSYS_CHANNEL5_RTC_PIT_DIV64_gc; // RTC/64 = 2ms pulse

   // Channel users
   EVSYS_USERCCLLUT2A  = EVSYS_USER_CHANNEL0_gc;  // LUT2-EVENTA  = Ch0 [Rx/Tx activity]
   EVSYS_USERCCLLUT2B  = EVSYS_USER_CHANNEL1_gc;  // LUT2-EVENTB  = Ch1 [XDIR]
   EVSYS_USERTCB0CAPT  = EVSYS_USER_CHANNEL2_gc;  // TCB0 Capture = Ch2 [LUT0-OUT=RxTx & ~XDIR]
   EVSYS_USERTCB1CAPT  = EVSYS_USER_CHANNEL3_gc;  // TCB1 Capture = Ch3 [LUT2-OUT=RxTx & XDIR]
   EVSYS_USERCCLLUT1A  = EVSYS_USER_CHANNEL4_gc;  // LUT1-EVENTA  = Ch4 [TCB1 Capt]
   EVSYS_USERTCB0COUNT = EVSYS_USER_CHANNEL5_gc;  // TCB0 Count   = Ch5 [2ms pulse]
   EVSYS_USERTCB1COUNT = EVSYS_USER_CHANNEL5_gc;  // TCB1 Count   = Ch5 [2ms pulse]

   //
   // Manage Rx pin (driven directly from the TCB0 output)
   //
   CCL_LUT0CTRLB = CCL_INSEL0_EVENTA_gc | CCL_INSEL1_EVENTB_gc; // EventA->INSEL0 / EventB->INSEL1
   CCL_LUT0CTRLC = 0; // Not used
   CCL_TRUTH0 = 0b100; // And of LUT0_IN1 and ~LUT0_IN0, LUT0-OUT = RTX & ~DIR
   CCL_LUT0CTRLA = CCL_ENABLE_bm;

   // Channel 2 (RTX & ~DIR) starts TCB0
   TCB0.CTRLB = TCB_CCMPEN_bm | TCB_ASYNC_bm | TCB_CNTMODE_SINGLE_gc; // Enable the output
   TCB0.CCMP = 5;                                 // 10ms pulse
   TCB0.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm; // Use the event channel as a clock source

   //
   // Manage Tx pin (driven directly from the LUT1 output, as TCB1 WO is not avail.)
   //
   CCL_LUT2CTRLB = CCL_INSEL0_EVENTA_gc | CCL_INSEL1_EVENTB_gc; // EventA->INSEL0 / EventB->INSEL1
   CCL_LUT2CTRLC = 0; // Not used
   CCL_TRUTH2 = 0b1000; // LUT2_OUT = LUT0_IN1 and LUT0_IN0
   CCL_LUT2CTRLA = CCL_ENABLE_bm;

   TCB1.CTRLB = TCB_ASYNC_bm | TCB_CNTMODE_SINGLE_gc; // No output
   TCB1.CCMP = 5;                                 // 10ms pulse
   TCB1.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm; // Use the event channel as a clock source

   // We need to take the timer output and route it through LUT1
   CCL_LUT1CTRLB = CCL_INSEL0_EVENTA_gc; // LUT1_IN0 = TCB0, LUT1_IN1 = None
   CCL_LUT1CTRLC = 0; // LUT1_IN2 = None
   CCL_TRUTH1 = 0b10; // OUT = EVENTA so the pin can be driven
   CCL_LUT1CTRLA = CCL_OUTEN_bm | CCL_ENABLE_bm; // Enable, and drive the output pin (PA7 - Tx)

   // Activate the CCL for both
   CCL_CTRLA = CCL_ENABLE_bm;
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
