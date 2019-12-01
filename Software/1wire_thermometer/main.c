/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 */

/*
 * Read out DS18B20 temperature sensor, and display measurement result
 * in flashing LEDs.
 *
 * Alternative firmware for:
 * https://www.schramm-software.de/bausatz/digitalthermometer/
 */

/* $Id$ */

#include "project.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <util/delay.h>

enum color
{
  RED, BLUE
};

uint16_t trigger_seconds;
#define TRIGGER_LEVEL 20       /* time between measurements, seconds */

static void go_to_sleep(void);
static void wake_up(void);
static volatile bool wdt_triggered, timer1_triggered;

ISR(WDT_vect)
{
  if (++trigger_seconds == TRIGGER_LEVEL)
    {
      trigger_seconds = 0;
      wdt_triggered = true;
      wake_up();
    }
  else
    {
      go_to_sleep();
    }
}

ISR(TIMER1_COMPA_vect)
{
  TIMSK1 &= ~_BV(OCIE1A); // disable interrupt
  TCCR1B = 0;             // stop timer
  timer1_triggered = true;
}

// long delay, for DS18B20 conversion time, and LED wait
void long_delay(uint16_t delay)
{
  TCNT1 = 0;
#if F_CPU == 12000000
  // scale down to 1.5 MHz while waiting
  clock_prescale_set(clock_div_8);
#endif
  // 1.5 MHz / 512 => 5.9 kHz, about 6 ticks per ms
  delay *= 6;
  OCR1A = delay;
  TCCR1B = _BV(CS12);
  TIFR1 = _BV(OCF1A); // clear any pending IRQ
  TIMSK1 |= _BV(OCIE1A);
  timer1_triggered = false;
  while (!timer1_triggered)
    sleep_mode();
#if F_CPU == 12000000
  // back to full speed
  clock_prescale_set(clock_div_1);
#endif
}

static void ledinit(void)
{
  DDRB |= _BV(LED_BLUE);
  DDRB |= _BV(LED_RED);
  DDRC |= _BV(OW_POWER_PIN);
  PORTB |= _BV(LED_BLUE); // LEDs off
  PORTB |= _BV(LED_RED);
  PORTD |= _BV(6);  // LED +
  DDRD |= _BV(6);
}

void ledflash(int count, enum color c)
{
  const uint8_t ledval =
  (c == RED)? _BV(LED_RED): _BV(LED_BLUE);

  while (count--)
    {
      PORTB &= ~ledval;
      long_delay(300);
      PORTB |= ledval;
      long_delay(300);
    }
}

int main(void)
{
  sei();
  ledinit();
#if F_CPU == 8000000 || F_CPU == 12000000
  clock_prescale_set(clock_div_1);
#elif F_CPU == 1000000
  clock_prescale_set(clock_div_8);
#else
#  error Cannot handle F_CPU value
#endif

  while(1)
    {
      PORTC |= _BV(OW_POWER_PIN);
      PORTB &= ~_BV(LED_BLUE);
      _delay_ms(10);
      PORTB |= _BV(LED_BLUE);
      double tfloat = Read_Temperature();
      PORTC &= ~_BV(OW_POWER_PIN);

      int t = (int)tfloat;

      div_t tdiv = div(abs(t), 10);
      if (tdiv.quot != 0)
        {
          // tens of degrees; only provide for |T| > 10 °C
          ledflash(tdiv.quot, t < 0? BLUE: RED);
          long_delay(400);
        }

      // ones of degrees; always sent
      int ones = tdiv.rem;
      if (ones == 0)
        ones = 10;
      ledflash(ones, t < 0? BLUE: RED);

      // fractional part; only sent if != .0; uses opposite color
      tfloat -= (double)t;
      tfloat *= 10.0;
      int fract = abs((int)tfloat);
      if (fract != 0)
        {
          long_delay(400);
          ledflash(fract, t < 0? RED: BLUE);
        }

      set_sleep_mode(SLEEP_MODE_PWR_DOWN);

      while (!wdt_triggered)
      {
        go_to_sleep();
        sleep_mode();
      }

      set_sleep_mode(SLEEP_MODE_IDLE);
      wdt_triggered = false;
    }
}

#pragma GCC optimize ("-Os")

static void
go_to_sleep(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  cli();
  wdt_reset();
  WDTCSR = _BV(WDCE) | _BV(WDE);
  WDTCSR = _BV(WDIF) | _BV(WDIE) | 6; /* 1 s interrupts */
  sei();
}

static void
wake_up(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);

  cli();
  wdt_reset();
  WDTCSR = _BV(WDCE) | _BV(WDE);
  WDTCSR = 0;           /* turn off */
  sei();
}
/* EOF */
