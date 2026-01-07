#pragma once
/**
 * Defines all I/Os of the board using PinDef
 * Compatible with C and C++ use (ioport.h or asx/ioport.hpp)
 * In C++, the namespace asx::ioport must be used
 */

/************************************************************************/
/* Functional I/Os                                                      */
/************************************************************************/
#define ISO_OUT_ES                  IOPORT(C, 3)
#define ISO_OUT_TOWER_LIGHT_RED     IOPORT(B, 3)
#define ISO_OUT_TOWER_LIGHT_YELLOW  IOPORT(B, 4)
#define ISO_OUT_TOWER_LIGHT_GREEN   IOPORT(B, 5)
#define ISO_OUT_RELEASE_STEPPER     IOPORT(C, 0)
#define ISO_OUT_LASER_CROSS         IOPORT(C, 1)
#define ISO_OUT_CAMERA_LIGHT        IOPORT(C, 2)

/************************************************************************/
/* Modbus LEDs                                                          */
/************************************************************************/
#define LED_MODBUS_RX               IOPORT(A, 5)
#define LED_MODBUS_TX               IOPORT(A, 3)

/************************************************************************/
/* I2C bus open collector for interrupt                                 */
/************************************************************************/
#define I2C_INT                     IOPORT(A, 6)

/************************************************************************/
/* Alert pin                                                            */
/************************************************************************/
#define ALERT_OUTPUT_PIN            IOPORT(A, 7)

/************************************************************************/
/* Debug pins                                                           */
/************************************************************************/
#define DEBUG_REACTOR_BUSY          IOPORT(A, 2)
