/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 */

/* $Id$ */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "thermo.h"

#include <util/crc16.h>
#include <util/delay.h>

#define concat(a, b) a##b
#define indir(a, b) concat(a, b)
#define ow_port indir(PORT, OW_PORT)
#define ow_pin  indir(PIN, OW_PORT)
#define ow_ddr  indir(DDR, OW_PORT)

static inline void
clear_dq(void)
{
  ow_port &= ~(1 << OW_PIN);
  ow_ddr  |= (1 << OW_PIN);
}

static inline void
set_dq(void)
{
  ow_ddr  &= ~(1 << OW_PIN);
  ow_port |= (1 << OW_PIN);
}

static inline void
strong_pullup(bool on)
{
  if (on)
  {
    ow_port |= (1 << OW_PIN);
    ow_ddr |= (1 << OW_PIN);
  }
  else
  {
    ow_port &= ~(1 << OW_PIN);
    ow_ddr &= ~(1 << OW_PIN);
  }
}

static inline uint8_t
get_dq(void)
{
  return (ow_pin & (1 << OW_PIN)) == 0;
}

void ow_power(bool on)
{
  ow_ddr |= (1 << OW_POWER_PIN);
  if (on)
    ow_port |= (1 << OW_POWER_PIN);
  else
    ow_port &= ~(1 << OW_POWER_PIN);
}

bool
ow_reset(void)
{
  bool presence;
  uint8_t saved_sreg = SREG;

  cli();
  clear_dq();
  _delay_ms(0.48);
  set_dq();
  _delay_ms(0.07);
  presence = get_dq();
  SREG = saved_sreg;
  _delay_ms(0.24);

  return presence;
}

static uint8_t
ow_read_bit(void)
{
  bool res;
  uint8_t saved_sreg = SREG;

  cli();
  clear_dq();
  set_dq();
  _delay_us(15);

  res = get_dq();
  SREG = saved_sreg;
  _delay_us(60);

  return res? 0: 1;
}

static void
ow_write_bit(bool bit)
{
  uint8_t saved_sreg = SREG;

  cli();
  clear_dq();
  _delay_us(1);
  if (bit)
    set_dq();
  _delay_ms(0.1);
  set_dq();
  _delay_us(1);
  SREG = saved_sreg;
}

static uint8_t
ow_read_byte(void)
{
  uint8_t i;
  uint8_t val = 0, mask;

  for (i = 0, mask = 1; i < 8; i++, mask <<= 1)
    {
      if (ow_read_bit())
        val |= mask;
      _delay_ms(0.1);
    }

  return val;
}

static void
ow_write_byte(uint8_t val)
{
  uint8_t i;
  uint8_t mask;

  for (i = 0, mask = 1; i < 8; i++, mask <<= 1)
    {
      ow_write_bit(val & mask);
    }
}

static uint8_t
ow_checkcrc(uint8_t *data, uint8_t nbytes)
{
  uint8_t crc = 0, i;

  for (i = 0; i < nbytes; i++)
    crc = _crc_ibutton_update(crc, data[i]);

  return crc; // must be 0
}

double
Read_Temperature(double divisor)
{
  uint8_t get[9];
  unsigned temp_lsb,temp_msb;
  uint8_t k;

  ow_reset();

  ow_write_byte(0xCC); // Skip ROM
  cli();
  ow_write_byte(0x44); // Start Conversion

  // strong pullup must be enabled within 10 µs after "Start
  // Conversion" => F_CPU = 8 MHz recommendable
  strong_pullup(true);
  sei();
  _delay_ms(100);
  strong_pullup(false);

  ow_reset();

  ow_write_byte(0xCC); // Skip ROM
  ow_write_byte(0xBE); // Read Scratch Pad

  for (k = 0; k < 9; k++){
    get[k] = ow_read_byte();
  }
  k = ow_checkcrc(get, 9);
  if (k != 0)
    return 0;

  temp_msb = get[1]; // Sign byte + lsbit
  temp_lsb = get[0]; // Temp data plus lsb
  int i = (temp_msb << 8) | temp_lsb;

  return ((double)i / divisor);
}

bool
Get_ROMCode(uint8_t *dat)
{
  uint8_t n;

  ow_reset();

  ow_write_byte(0x33);

#if UART_DEBUG
  putstring("ROM Code: ");
#endif
  for (n = 0; n < 8; n++) {
    dat[n] = ow_read_byte();
#if UART_DEBUG
    char b[10];
    sprintf(b, "0x%02x ", dat[n]);
    putstring(b);
#endif
  }
  n = ow_checkcrc(dat, 8);
#if UART_DEBUG
  if (n)
    putstring("CRC fail\r\n");
  else
    putstring("CRC OK\r\n");
#endif
  return n == 0;
}

void
Configure_DS18B20(void)
{
  uint8_t get[9];
  uint8_t k;

  ow_reset();

  ow_write_byte(0xCC); // Skip ROM
  ow_write_byte(0xBE); // Read Scratch Pad

  for (k = 0; k < 9; k++) {
    get[k] = ow_read_byte();
  }
  k = ow_checkcrc(get, 9);
  if (k != 0)
    return;

  get[4] = 0x1F; // configuration register, R1 = R0 = 0, 9-bit operation

  ow_write_byte(0x4E); // Write Scratch Pad
  for (k = 2; k < 5; k++) {
    ow_write_byte(get[k]);
  }

  ow_reset();
}
