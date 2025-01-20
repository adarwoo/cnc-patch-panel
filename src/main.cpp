/*
 * Console modbus device main entry point.
 */
#include <asx/reactor.hpp>
#include "conf_modbus.hpp"

int main()
{
   patch::modbus_master::init();

   // Run the reactor/scheduler
   asx::reactor::run();
}
