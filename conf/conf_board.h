#pragma once

/************************************************************************/
/* Functional I/Os                                                      */
/************************************************************************/
#define ISO_OUT_ES                  asx::ioport::PinDef(asx::ioport::B, 2)
#define ISO_OUT_TOWER_LIGHT_RED     asx::ioport::PinDef(asx::ioport::B, 3)
#define ISO_OUT_TOWER_LIGHT_YELLOW  asx::ioport::PinDef(asx::ioport::B, 4)
#define ISO_OUT_TOWER_LIGHT_GREEN   asx::ioport::PinDef(asx::ioport::A, 5)
#define ISO_OUT_RELEASE_STEPPER     asx::ioport::PinDef(asx::ioport::C, 0)
#define ISO_OUT_LASER_CROSS         asx::ioport::PinDef(asx::ioport::C, 1)
#define ISO_OUT_CAMERA_LIGHT        asx::ioport::PinDef(asx::ioport::C, 2)

// Modbus LEDs
#define LED_MODBUS_RX               asx::ioport::PinDef(asx::ioport::A, 7)
// NOTE: Requires mod to PCB. Originally connected to the i2c INT pin 
#define LED_MODBUS_TX               asx::ioport::PinDef(asx::ioport::A, 5)

// I2C bus open collector for interrupt
#define I2C_INT                     asx::ioport::PinDef(asx::ioport::A, 6)

// Alert pin
#define ALERT_OUTPUT_PIN            IOPORT_CREATE_PIN(PORTA, 3)

// Debug pins
#define DEBUG_REACTOR_IDLE          IOPORT_CREATE_PIN(PORTA, 2)
