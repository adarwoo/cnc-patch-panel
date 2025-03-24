#pragma once

/************************************************************************/
/* Functional I/Os                                                      */
/************************************************************************/
#define ISO_OUT_ES                  asx::ioport::B, 2
#define ISO_OUT_TOWER_LIGHT_RED     asx::ioport::B, 3
#define ISO_OUT_TOWER_LIGHT_YELLOW  asx::ioport::B, 4
#define ISO_OUT_TOWER_LIGHT_GREEN   asx::ioport::A, 5
#define ISO_OUT_RELEASE_STEPPER     asx::ioport::C, 0
#define ISO_OUT_LASER_CROSS         asx::ioport::C, 1
#define ISO_OUT_LASER_CAMERA_LIGHT  asx::ioport::C, 2

// Alert pin
#define ALERT_OUTPUT_PIN            IOPORT_CREATE_PIN(PORTA, 3)

// Debug pins
#define DEBUG_REACTOR_IDLE          IOPORT_CREATE_PIN(PORTA, 2)
