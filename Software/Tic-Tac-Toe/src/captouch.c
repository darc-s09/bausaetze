
/* === includes ============================================================ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "spiel.h"


/* === macros ============================================================== */
#define NSAMPLES (8) /* must match the inline asm below! */
#define NSAMPLE  (2)
#define KEY_NONE (255)

#define KEY_RELEASED (0)
#define KEY_PRESSED (1)

/* === globals ============================================================= */
static uint8_t SamplesPortB[NSAMPLES], SamplesPortG[NSAMPLES];
static uint8_t PadState[9] = { KEY_RELEASED };

/* === functions =========================================================== */

/*
 * Sample port B's input pins for their attached capacitance.
 *
 * Only bit 0..2 are used for input pads.
 *
 * First, the port is set to output, at low level, to discharge the
 * capacitor attached to the pin.  Then, the port is turned into an
 * input, and the input pullups are applied.  This causes the input
 * capacitor to be slowly charged, while the digital input register is
 * continuously sampled 16 times.  As the timing of this part is
 * crucial, inline assembly is used to quickly (and in equal time
 * steps) sample the input data into registers, from where they can be
 * stored into their final destination later on (by the compiler).
 */
static void sample_port_b(void)
{
    uint8_t portb = PORTB;

    portb &= ~0x07;
    PORTB = portb;
    portb |= 0x07;
    DDRB |= 0x07;
    __asm ("nop");
    DDRB &= ~0x07;
    PORTB = portb;
    __asm ("in %[s0], %[pinb]" "\n\t"
           "in %[s1], %[pinb]" "\n\t"
           "in %[s2], %[pinb]" "\n\t"
           "in %[s3], %[pinb]" "\n\t"
           "in %[s4], %[pinb]" "\n\t"
           "in %[s5], %[pinb]" "\n\t"
           "in %[s6], %[pinb]" "\n\t"
           "in %[s7], %[pinb]"
           :
           /* output operands */
           [s0] "=r" (SamplesPortB[0]),
           [s1] "=r" (SamplesPortB[1]),
           [s2] "=r" (SamplesPortB[2]),
           [s3] "=r" (SamplesPortB[3]),
           [s4] "=r" (SamplesPortB[4]),
           [s5] "=r" (SamplesPortB[5]),
           [s6] "=r" (SamplesPortB[6]),
           [s7] "=r" (SamplesPortB[7])
           :
           /* input operands */
           [pinb] "I" (_SFR_IO_ADDR(PINB)));
}

/*
 * Same as for port B above, but port G has only 6 bit.
 */
static void sample_port_g(void)
{
    PORTG = 0;
    DDRG = 0x3f;
    __asm ("nop");
    DDRG = 0;
    PORTG = 0x3f;
    __asm ("in %[s0], %[ping]" "\n\t"
           "in %[s1], %[ping]" "\n\t"
           "in %[s2], %[ping]" "\n\t"
           "in %[s3], %[ping]" "\n\t"
           "in %[s4], %[ping]" "\n\t"
           "in %[s5], %[ping]" "\n\t"
           "in %[s6], %[ping]" "\n\t"
           "in %[s7], %[ping]"
           :
           /* output operands */
           [s0] "=r" (SamplesPortG[0]),
           [s1] "=r" (SamplesPortG[1]),
           [s2] "=r" (SamplesPortG[2]),
           [s3] "=r" (SamplesPortG[3]),
           [s4] "=r" (SamplesPortG[4]),
           [s5] "=r" (SamplesPortG[5]),
           [s6] "=r" (SamplesPortG[6]),
           [s7] "=r" (SamplesPortG[7])
           :
           /* input operands */
           [ping] "I" (_SFR_IO_ADDR(PING)));
}

uint8_t update_pads(uint8_t dummy)
{
    uint8_t scans[9], i, ret;

    ret = KEY_NONE;
    sample_port_b();
    sample_port_g();

    for (i=0; i<9; i++)
    {
        switch(i)
        {
            case 0:
                scans[0] = (SamplesPortB[NSAMPLE] & _BV(PB2)) ;
                break;
            case 1:
                scans[1] = (SamplesPortG[NSAMPLE] & _BV(PG5)) ;
                break;
            case 2:
                scans[2] = (SamplesPortG[NSAMPLE] & _BV(PG2)) ;
                break;
            case 3:
                scans[3] = (SamplesPortB[NSAMPLE] & _BV(PB1)) ;
                break;
            case 4:
                scans[4] = (SamplesPortG[NSAMPLE] & _BV(PG4)) ;
                break;
            case 5:
                scans[5] = (SamplesPortG[NSAMPLE] & _BV(PG1)) ;
                break;
            case 6:
                scans[6] = (SamplesPortB[NSAMPLE] & _BV(PB0)) ;
                break;
            case 7:
                scans[7] = (SamplesPortG[NSAMPLE] & _BV(PG3)) ;
                break;
            case 8:
                scans[8] = (SamplesPortG[NSAMPLE] & _BV(PG0)) ;
                break;
        }

        /* detect key events */
        if ((scans[i] > 0) && (PadState[i] != KEY_RELEASED))
        {
            PadState[i] = KEY_RELEASED;
        }

        if ((scans[i] == 0) && (PadState[i] != KEY_PRESSED))
        {
            PadState[i] = KEY_PRESSED;
            ret = i;
            __asm ("nop");
            /* verlasse Schleife bei der ersten gedrückten Taste */
            break;
        }
    }

    return ret;
}

