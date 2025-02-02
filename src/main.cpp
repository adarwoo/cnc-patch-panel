/*
 * Console modbus device main entry point.
 */
#include <asx/reactor.hpp>
#include "patch.hpp"


int main()
{
   // Initialse the patch management (i2c, gpio and modbus sequencer)
   patch::init();

   // Run the reactor/scheduler
   asx::reactor::run();
}
