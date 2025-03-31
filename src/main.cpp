/*
 * Console modbus device main entry point.
 */
#include <trace.h>
#include <asx/reactor.hpp>
#include "patch.hpp"
#include "door.hpp"

int main()
{
   TRACE_MILE(APP, "CNC Patch starting");

   // Initialse the patch management (i2c, gpio and modbus sequencer)
   patch::init();

   // Initialse the door controller
   door::init();

   // Run the reactor/scheduler
   asx::reactor::run();
}
