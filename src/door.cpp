#include <chrono>
#include <boost/sml.hpp>

#include <asx/reactor.hpp>

#include "iomux.hpp"
#include "modbus.hpp"

namespace door
{
    using namespace std::chrono;
    using namespace asx;
    namespace sml = boost::sml;

    // Constants
    constexpr auto moving_timeout = 3s;
    constexpr auto complete_timeout = 8s;

    // Locals
    timer::Instance timer = timer::null;
    reactor::Handle react_on_timeout = reactor::null;

    // Create those simple events
    struct event_open             {};
    struct event_close            {};
    struct event_door_is_up       {};
    struct event_door_is_down     {};
    struct event_door_moving_up   {};
    struct event_door_moving_down {};
    struct event_timeout          {};

    /************************************************************************/
    /* State machine table                                                  */
    /************************************************************************/
    struct DoorSm {
        auto operator()() const noexcept {
            using namespace sml;

            auto push_on = [] {
                modbus::coils.door_push = 1;
                iomux::led::set(iomux::led::Id::door_opening, true);
                timer = react_on_timeout.delay(moving_timeout);
            };
        
            auto push_off = [] {
                modbus::coils.door_push = 0;
                iomux::led::set(iomux::led::Id::door_opening, false);
                timer.cancel();
            };
        
            auto push_timeout = [] {
                modbus::coils.door_push = 0;
                iomux::led::blink(iomux::led::Id::door_opening);
            };
        
            auto pull_on = [] {
                modbus::coils.door_pull = 1;
                iomux::led::set(iomux::led::Id::door_closing, true);
                timer.cancel();
            };
        
            auto pull_off = [] {
                modbus::coils.door_pull = 0;
                iomux::led::set(iomux::led::Id::door_closing, false);
                timer.cancel();
            };
        
            auto pull_timeout = [] {
                modbus::coils.door_pull = 0;
                iomux::led::blink(iomux::led::Id::door_closing);
            };
        
            auto door_moving = [] {
                timer.cancel();
                timer = react_on_timeout.delay(complete_timeout);
            };
    
            return make_transition_table(
               *"unknown"_s + event<event_door_is_up>                      = "opened"_s,
                "unknown"_s + event<event_door_is_down>                    = "closed"_s,
                "closed"_s  + event<event_open>             / push_on      = "opening"_s,
                "opened"_s  + event<event_close>            / pull_on      = "closing"_s,
                "opening"_s + event<event_door_moving_up>   / door_moving  = "opening"_s,
                "opening"_s + event<event_timeout>          / push_timeout = "unknown"_s,
                "opening"_s + event<event_door_is_up>       / push_off     = "opened"_s,
                "closing"_s + event<event_door_moving_down> / door_moving  = "closing"_s,
                "closing"_s + event<event_timeout>          / pull_timeout = "unknown"_s,
                "closing"_s + event<event_door_is_down>     / pull_off     = "closed"_s
            );
        }
    };

    ///< The overall modbus state machine
    static auto door_sm = boost::sml::sm<DoorSm>{};

    // React to timeouts
    void on_timeout() {
        door_sm.process_event(event_timeout{});
    }

    /**
     * Reactor handler invoked from the iomux when the door command changes
     * Pass to the state machine to handle the request
     */
    void on_check_inputs() {
        // Store the button status
        static bool open_close = false;

        // Store the sensor state too
        static bool is_closed = false;
        static bool is_opened = false;

        // Compare to detect a change
        if ( open_close != iomux::inputs.door_open_close ) {
            open_close = !open_close;

            // Pump the status into the state machine
            if (open_close) {
                door_sm.process_event(event_open{});
            } else {
                door_sm.process_event(event_close{});
            }
        }

        // Update the sensor state
        if ( is_opened != iomux::inputs.door_is_up ) {
            is_opened = !is_opened;

            if (is_opened) {
                door_sm.process_event(event_door_is_up{});
            } else {
                door_sm.process_event(event_door_moving_down{});
            }
        } else if ( is_closed != iomux::inputs.door_is_down ) {
            is_closed = !is_closed;

            if (is_closed) {
                door_sm.process_event(event_door_moving_up{});
            } else {
                door_sm.process_event(event_door_is_down{});
            }
        }
    }

    /** Call once */
    void init() {
        using namespace std::chrono;

        // Register the timeout handler
        react_on_timeout = reactor::bind(on_timeout);

        // Start sampling the inputs
        reactor::bind(on_check_inputs).repeat(100ms);
    }
};    
