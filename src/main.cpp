/*
 * Console modbus device main entry point.
 */
#include <trace.h>
#include <asx/reactor.hpp>
#include "patch.hpp"

int main()
{
   TRACE_MILE(PATCH, "CNC Patch starting");

   // Initialse the patch management (i2c, gpio and modbus sequencer)
   patch::init();

   // Run the reactor/scheduler
   asx::reactor::run();
}
