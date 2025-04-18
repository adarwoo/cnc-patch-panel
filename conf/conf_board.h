#pragma once
/**
 * Defines all I/Os of the board using PinDef
 * Compatible with C and C++ use (ioport.h or asx/ioport.hpp)
 * In C++, the namespace asx::ioport must be used
 */

/************************************************************************/
/* Functional I/Os                                                      */
/************************************************************************/
#define ISO_OUT_ES                  PinDef(B, 2)
#define ISO_OUT_TOWER_LIGHT_RED     PinDef(B, 3)
#define ISO_OUT_TOWER_LIGHT_YELLOW  PinDef(B, 4)
#define ISO_OUT_TOWER_LIGHT_GREEN   PinDef(B, 5)
#define ISO_OUT_RELEASE_STEPPER     PinDef(C, 0)
#define ISO_OUT_LASER_CROSS         PinDef(C, 1)
#define ISO_OUT_CAMERA_LIGHT        PinDef(C, 2)

/************************************************************************/
/* Modbus LEDs                                                          */
/************************************************************************/
#define LED_MODBUS_RX               PinDef(A, 5)
#define LED_MODBUS_TX               PinDef(A, 3)

/************************************************************************/
/* I2C bus open collector for interrupt                                 */
/************************************************************************/
#define I2C_INT                     PinDef(A, 6)

/************************************************************************/
/* Alert pin                                                            */
/************************************************************************/
#define ALERT_OUTPUT_PIN            PinDef(A, 7)

/************************************************************************/
/* Debug pins                                                           */
/************************************************************************/
#define DEBUG_REACTOR_BUSY          PinDef(A, 2)
#define DEBUG_TRACE                 PinDef(C, 3)

// To quick debug with trace pin
#define TRACE_ON                    {asm("sbi 9,3");}
#define TRACE_OFF                   {asm("cbi 9,3");}
