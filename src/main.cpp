/*
 * Console modbus device main entry point.
 */
#include <chrono>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <trace.h>
#include <asx/ioport.hpp>
#include <asx/reactor.hpp>

#include "patch.hpp"
#include "door.hpp"
#include "conf_board.h"

using namespace asx::ioport;


/**
 * Combine the event system with the configurable custom logic to drive
 * the UART LEDs without software.
 * The timer B0 is uses to create a visible pulse. The timer starts when a rising edge
 *  is detected on the Rx/Tx pin. The direction pin is used to drive Tx or Rx.
 * This toggles the flip flop and the timer resets the flip flop.
 * We use the PIT timer which we assume is running at 32768Hz to feed the timerB0
 */
void setup_modbus_activity_leds() {
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"

   // Set the pin direction to output
   Pin(LED_MODBUS_RX).init(dir_t::out);
   Pin(LED_MODBUS_TX).init(dir_t::out);

   // Channel 0 reports Rx/Tx activity
   EVSYS_CHANNEL0 = EVSYS_CHANNEL0_PORTA_PIN1_gc;

   // Channel 1 reports XDIR direction selection
   EVSYS_CHANNEL1 = EVSYS_CHANNEL1_PORTA_PIN4_gc;

   // Channel 2 reports the timer TCB1 pulse
   EVSYS_CHANNEL2 = EVSYS_CHANNEL2_TCB1_CAPT_gc;

   // Channel 3 reports the output of the LUT0 (RTX & ~XDIR)
   EVSYS_CHANNEL3 = EVSYS_CHANNEL3_CCL_LUT0_gc;

   // Channel 4 reports the output of the LUT2 (RTX & XDIR)
   EVSYS_CHANNEL4 = EVSYS_CHANNEL4_CCL_LUT2_gc;

   // Channel 5 (Odd channel) feeds from the RTC clock (configured at 32k) - so 2ms pulse
   EVSYS_CHANNEL5 = EVSYS_CHANNEL5_RTC_PIT_DIV64_gc;

   // Intputs of LUT0 is channel 0 and channel 1 (and NC)
   EVSYS_USERCCLLUT0A = EVSYS_USER_CHANNEL0_gc; // XDIR
   EVSYS_USERCCLLUT0B = EVSYS_USER_CHANNEL1_gc; // Rx/Tx
   CCL_LUT0CTRLB = CCL_INSEL0_EVENTA_gc | CCL_INSEL1_EVENTB_gc; // EventA->INSEL0 / EventB->INSEL1
   CCL_LUT0CTRLC = 0; // Not used
   CCL_TRUTH0 = 0b100; // And of LUT0_IN1 and ~LUT0_IN0, LUT0-OUT = RTX & ~DIR
   CCL_LUT0CTRLA = CCL_ENABLE_bm;

   // Channel 3 (RTX & ~DIR) starts TCB1
   EVSYS_USERTCB1CAPT = EVSYS_USER_CHANNEL3_gc;   // Channel 3 (LUT0) starts a single shot
   EVSYS_USERTCB1COUNT = EVSYS_USER_CHANNEL5_gc;  // Channel 5 (RTC Clock)
   TCB1.CTRLA = TCB_CLKSEL_EVENT_gc;              // Use the event channel as a clock source
   TCB1.CTRLB = TCB_ASYNC_bm | TCB_CNTMODE_SINGLE_gc;
   TCB1.CCMP = 5;                                 // 10ms pulse
   TCB1.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm;

   // Intputs of LUT1 is channel 2 (Output of TCB1)
   EVSYS_USERCCLLUT1A = EVSYS_USER_CHANNEL2_gc;  // EVENTA of LUT1 is Event channel 2 (TCB0)
   CCL_LUT1CTRLB = CCL_INSEL0_EVENTA_gc; // LUT1_IN0 = TCB0, LUT1_IN1 = None
   CCL_LUT1CTRLC = 0; // LUT1_IN2 = None
   CCL_TRUTH1 = 0b10; // Straigth so the pin can be driven
   CCL_LUT0CTRLA = CCL_OUTEN_bm | CCL_ENABLE_bm; // Enable, and drive the output pin (PA7 - Tx)

   // Intputs of LUT2 is channel 0 and channel 1 (and NC)
   EVSYS_USERCCLLUT2A = EVSYS_USER_CHANNEL0_gc; // XDIR
   EVSYS_USERCCLLUT2B = EVSYS_USER_CHANNEL1_gc; // Rx/Tx
   CCL_LUT2CTRLB = CCL_INSEL0_EVENTA_gc | CCL_INSEL1_EVENTB_gc; // EventA->INSEL0 / EventB->INSEL1
   CCL_LUT2CTRLC = 0; // Not used
   CCL_TRUTH2 = 0b1000; // And of LUT0_IN1 and LUT0_IN0, LUT3-OUT
   CCL_LUT2CTRLA = CCL_ENABLE_bm; 

   // Channel 4 (RTX & ~DIR) starts TCB0
   EVSYS_USERTCB1CAPT = EVSYS_USER_CHANNEL4_gc;   // Channel 3 (LUT0) starts a single shot
   EVSYS_USERTCB1COUNT = EVSYS_USER_CHANNEL5_gc;  // Channel 5 (RTC Clock)
   TCB0.CTRLA = TCB_CLKSEL_EVENT_gc;              // Use the event channel as a clock source
   TCB0.CTRLB = TCB_ASYNC_bm | TCB_CNTMODE_SINGLE_gc;
   TCB0.CCMP = 5;                                 // 10ms pulse
   TCB0.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_CCMPEN_bm | TCB_ENABLE_bm; // Connects the TB0 Output to PA5
   
   // Activate the CCL
   CCL_CTRLA = CCL_ENABLE_bm;
}


int main()
{
   TRACE_MILE(APP, "CNC Patch starting");

   // Initialse the patch management (i2c, gpio and modbus sequencer)
   patch::init();

   // Enable monitoring the activities
   setup_modbus_activity_leds();

   // Initialse the door controller
   door::init();

   // Run the reactor/scheduler
   asx::reactor::run();
}
