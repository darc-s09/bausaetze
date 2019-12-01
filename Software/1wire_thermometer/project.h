/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 */

/* $Id$ */

#include <stdbool.h>
#include <stdint.h>

/* LEDs */
#define LED_BLUE        0 /* PB0 */
#define LED_RED         1 /* PB1 */

/* PB2: LED R */

/* one-wire IC powerup */
#define OW_POWER_PIN    3

/* one-wire port DS18B20 */
#define OW_PORT         C
#define OW_PIN          2

extern bool ow_reset(void);
extern void Print_ROMCode(void);
extern double Read_Temperature(void);
extern void long_delay(uint16_t);
