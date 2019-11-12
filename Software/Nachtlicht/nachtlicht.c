/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Joerg Wunsch wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define F_CPU 1200000ul
#include <util/delay.h>

// measured times in milliseconds for "bright" and "fully dark"
static const uint16_t bright_light = 200;
static const uint16_t dark_light = 500;
// maximal PWM value to use (to save power)
static const uint8_t max_pwm = 150;

static uint16_t seconds;
static uint16_t interval = 10; // measuring interval during daylight

// timer 0 ticks, about 1.7 ms each
static volatile uint16_t ticks;

// action flags for main loop
static volatile bool start_measurement;
static volatile bool update_led;

// current LED status
static bool led_is_on;

static inline uint16_t ticks_to_ms(uint16_t t)
{
  return (256ul * 8 * 1000) * t / F_CPU;
}


// Timer 0 serves two purposes:
//
// First, it operates a PWM to dim the LED, which is connected to OC0A
// (PB0). To minimize flicker, the prescaler is set to 8, resulting in
// about 580 Hz of PWM frequency.
//
// Second, it is used to measure the capacitor discharge time of the
// ambient light sensor.
//
// If none of both functions is needed, timer 0 can be turned off, and
// the controller might go to powerdown sleep. If timer 0 needs to
// run, the controller is only allowed to got to idle sleep.
static void start_timer0(void)
{
  // fast PWM, set OC0A on TOP, clear on compare match
  TCCR0A = _BV(WGM01) | _BV(WGM00) |
  // only activate OC0A if LED is on - otherwise it will yield
  // a 1-clock (7 µs) pulse resulting in an unwanted dim LED
  (led_is_on? _BV(COM0A1): 0);
  // prescaler 8 => 150 kHz timer clock
  TCCR0B = _BV(CS01);
  // enable overflow interrupt
  TIMSK0 = _BV(TOIE0);
}

static void stop_timer0(void)
{
  TCCR0B = 0;
  // disable PWM outputs, too, so LED is off
  TCCR0A = 0;
}

// Trigger measurement of ambient light.
//
// The measurement principle is to charge a 100 nF capacitor, and have
// it discharged by a photo diode, operated in reverse polarity (anode
// on GNC, cathode on PB1). The discharge process is monitored by the
// analog comparator against the internal 1.1 V bandgap reference,
// which then triggers an interrupt when falling below the bandgap
// voltage. This is used to take the time from Timer 0.  The time this
// takes is less than 10 ms for bright light, about 50 ms for dim
// light, up to several 100 ms in total darkness.
static void measure(void)
{
  // Charge capacitor on PB1 (= AIN1).
  DDRB |= _BV(1);
  PORTB |= _BV(1);

  ticks = 0;
  start_timer0();

  // Enable analog comparator, and bandgap reference.  Clear possibly
  // pending interrupt, too.  Have the interrupt trigger on positive
  // edge.
  ACSR = _BV(ACBG) | _BV(ACI) | _BV(ACIS1) | _BV(ACIS0);

  // With a pin driver strength of 10 mA, charging 100 nF takes about
  // 30 µs.  50 µs gets us on the safe side.
  _delay_us(50);

  // now, enable the interrupt
  ACSR = _BV(ACBG) | _BV(ACI) | _BV(ACIS1) | _BV(ACIS0);
  ACSR |= _BV(ACIE);

  // Release pin again, so the photo diode can start discharging
  // the capacitor, depending on the available light.
  PORTB &= ~_BV(1);
  DDRB &= ~_BV(1);

  // keep system clock running by now
  set_sleep_mode(SLEEP_MODE_IDLE);
}

// Watchdog interrupt is triggered approximately each second.  Used to
// count up seconds. If the measurement interval has elapsed, trigger
// a new light measurement, otherwise go to sleep again immediately.
ISR(WDT_vect)
{
  if (!led_is_on)
    {
      static uint16_t previous;

      uint16_t now = ++seconds;
      // Unsigned integer arithmetics is guaranteed to roll over in a
      // well-defined way, so we can simply subtract values to get a valid
      // time difference even in case of a roll over from 65535 to 0.
      uint16_t delta = now - previous;
      if (delta > interval)
        {
          previous = now;
          start_measurement = true;
        }
    }
}

// Timer overflow interrupt, triggers each 1.7 ms. Just count up.
ISR(TIM0_OVF_vect)
{
  // use local shadow variable for volatile global "ticks"
  uint16_t now = ticks;
  now++;
  ticks = now;

  if (ticks_to_ms(now) > dark_light)
    // Measurement time exceeds the one for darkness - abort
    // measurement now by discharching the capacitor (this will
    // trigger the ANA_COMP interrupt immediately)
    DDRB |= _BV(1);
}

// Analog comparator interrupt: end of ambient light measurement
//
// Take total time, and decide what to do.
ISR(ANA_COMP_vect)
{
  // discharge 100 nF C
  DDRB |= _BV(1);
  // turn off analog comparator
  ACSR = _BV(ACD);

  update_led = true;
}

int
main(void)
{
    // PB1 / AIN1 is analog input only
    DIDR0 = _BV(AIN1D);

    // start watchdog timer, as 1 s clock
    WDTCR = _BV(WDTIF);
    WDTCR = _BV(WDP2) | _BV(WDP1) | _BV(WDTIE);

    DDRB = _BV(0); // LED output
    PORTB = _BV(2) | _BV(3) | _BV(4); // pullups on jumper and SCK

    sei();

    for (;;)
    {
      if (update_led)
        {
          update_led = false;

          uint16_t ms = ticks_to_ms(ticks);

          if (ms < bright_light)
            {
              // it's bright light, turn LED off
              OCR0A = 0;
              stop_timer0();
              led_is_on = false;
            }
          else
            {
              // turn LED on
              if (ms > dark_light)
                OCR0A = max_pwm;
              else
                OCR0A = (ms - bright_light) * max_pwm / (dark_light - bright_light);
              led_is_on = true;
            }

          if (!led_is_on)
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);
          else
            start_measurement = true;
        }

      if (start_measurement)
        {
          start_measurement = false;
          measure();
        }
      sleep_mode();
    }

    return 0;
}
