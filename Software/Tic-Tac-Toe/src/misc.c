/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "spiel.h"

/* === macros ============================================================== */
/* === types =============================================================== */
/* === globals ============================================================= */
static volatile int16_t SleepCounter;
static volatile uint16_t TmoCounter;
static volatile uint8_t LocalKey;
static volatile error_t ErrorState;

static uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

/* === functions =========================================================== */

/* === Timer init ========================================================== */
void xxo_io_init(void)
{
    /*
     * Timer 0 with prescaler 8, and overflow interrupts enabled.
     * At F_CPU = 1 MHz, the timer rolls over at 488 Hz, or
     * approximately each 2 ms.
     */
    TCCR0B = _BV(CS01);
    TIMSK0 = _BV(TOIE0);
    OCR0A = 220;
    sei();

    /*
     * Pin driver strength port B (LED port) = 8 mA
     */
    DPDS0 = _BV(PBDRV1) | _BV(PBDRV0);
}
/* === Timer Funktionen ==================================================== */
void xxo_set_timeout(uint16_t tmo)
{
    cli();
    TmoCounter = (tmo >> 1);
    sei();
}


ISR(TIMER0_OVF_vect)
{
    static uint8_t row = 0;
    uint8_t key;

    if (SleepCounter > 0)
    {
        SleepCounter--;
    }

    /* update timeout counter */
    TmoCounter --;
    if (TmoCounter == 0)
    {
        xxo_set_error(TIMEOUT);
    }


    key = update_pads(row);

    if ((key != KEY_NONE) && (LocalKey == KEY_NONE))
    {
        LocalKey = key;
    }

    display_leds(row);

    row ++;
    if (row > 2)
    {
        row = 0;
    }
}

void xxo_sleep(int16_t sleep_time)
{
    set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    SleepCounter = (sleep_time>>1);
    while (SleepCounter > 0)
    {
        sei();
        sleep_mode();
        cli();
    }
    sei();
}

/* === Watchdog Funktionen ================================================ */

/*
 * Sicherstellen, dass der Watchdog nach einem Watchdog-Reset wirklich
 * ruhig gestellt wird.  Siehe Datenblatt 13.4.2 (Seite 184 in
 * Dokument 8266C-MCU Wireless-08/11).
 *
 * Diese Funktion wird nicht mittels CALL gerufen, sondern vom Linker
 * in den Startup-Code eingebaut.  Daher ist sie "naked"
 * (d. h. bekommt weder Prolog noch Epilog, nicht einmal ein RET am
 * Ende) und wird in Section .init3 platziert.  Sie ist nur aus Sicht
 * der C-Syntax eine Funktion, da sich in C ausführbarer Code stets
 * innerhalb einer Funktion befinden muss.
 */
void get_mcusr(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void get_mcusr(void)
{
  mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}

uint8_t xxo_is_power_on_reset(void)
{
    return (mcusr_mirror & _BV(PORF));
}

ISR(WDT_vect)
{
    uint8_t key;

    /* Wir sind nur an Taste 4 interessiert, die in Reihe 1 liegt. */
    key = update_pads(1);

    if (key == 4)
    {
        LocalKey = key;

        /* 15 ms Timeout, Watchdog im Reset-Mode */
        wdt_enable(WDTO_15MS);
        for (;;)
        {
            /* Warten auf den Watchdog-Reset */
        }
    }
}


/* === Fehlerbehandlung ==================================================== */
void xxo_set_error(error_t error)
{
    cli();
    ErrorState = error;
    sei();
}

error_t xxo_get_error(void)
{
    error_t ret;

    cli();
    ret = ErrorState;
    ErrorState = NO_ERROR;
    sei();
    return ret;
}

/* === Tastatur Abfrage ==================================================== */
uint8_t xxo_get_key(void)
{
uint8_t ret;
    cli();
    ret = LocalKey;
    LocalKey = KEY_NONE;
    sei();
    return ret;
}

/* === Tiefschlaf ========================================================== */
/*
 * Diese Funktion muss als letzte stehen, da das Pragma für den Rest
 * der Datei die Optimierung ändert.  Auf diese Weise kann man bei
 * Bedarf trotzdem noch alles mit -O0 (nicht optimiert) compilieren,
 * nur xxo_deep_sleep() wird immer optimiert.
 */
#pragma GCC optimize ("-Os")
/*
 * Diese Funktion muss immer optimiert compiliert werden, damit das
 * Timing des WDCE-Bits eingehalten wird.
 */
void xxo_deep_sleep(void)
{
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    PORTB &= ~(COL_LEDS | ROW_IO_MASK);
    DDRB = COL_LEDS | ROW_IO_MASK;
    PORTG = 0;
    DDRG = 0x3f;                /* kapazitive Touch-Pads */
    PORTD &= ~COL_PADS;
    DDRD = COL_PADS;

    // ungenutzte Portpins
    DDRE = 0xff;
    PORTE = 0;
    DDRF = 0xff;
    PORTF = 0;
    PORTD |= 0x0f;              /* Nutzer-Pins: I²C + UART 1 */
    PORTB |= 0xc0;

    cli();
    wdt_reset();
    WDTCSR = _BV(WDCE) | _BV(WDE);
    WDTCSR = _BV(WDIF) | _BV(WDIE) | 5; /* 500 ms interrupts */
    sei();
    TCCR0B = 0;

    sleep_mode();
}


