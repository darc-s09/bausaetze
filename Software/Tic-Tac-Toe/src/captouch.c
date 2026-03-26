
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
static uint8_t SamplesPortD[NSAMPLES], SamplesPortG[NSAMPLES];
static uint8_t PadState[9] = { KEY_RELEASED };

/* === functions =========================================================== */

/*
 * Sample port D's input pins for their attached capacitance.
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
static void sample_port_d(void)
{
    PORTD = 0;
    DDRD = 0xe0;
    __asm ("nop");
    DDRD = 0;
    PORTD = 0xe0;
    __asm ("in %[s0], %[pind]" "\n\t"
           "in %[s1], %[pind]" "\n\t"
           "in %[s2], %[pind]" "\n\t"
           "in %[s3], %[pind]" "\n\t"
           "in %[s4], %[pind]" "\n\t"
           "in %[s5], %[pind]" "\n\t"
           "in %[s6], %[pind]" "\n\t"
           "in %[s7], %[pind]"
           :
           /* output operands */
           [s0] "=r" (SamplesPortD[0]),
           [s1] "=r" (SamplesPortD[1]),
           [s2] "=r" (SamplesPortD[2]),
           [s3] "=r" (SamplesPortD[3]),
           [s4] "=r" (SamplesPortD[4]),
           [s5] "=r" (SamplesPortD[5]),
           [s6] "=r" (SamplesPortD[6]),
           [s7] "=r" (SamplesPortD[7])
           :
           /* input operands */
           [pind] "I" (_SFR_IO_ADDR(PIND)));
}

/*
 * Same as for port D above, but only portpin G0 is used as an input,
 * all other pins are not used for that purpose (but are sampled
 * anyway, as sampling always applies to a full port).
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
    sample_port_d();
    sample_port_g();

    for (i=0; i<9; i++)
    {
        switch(i)
        {
            case 0:
                scans[0] = (SamplesPortD[NSAMPLE] & _BV(PD7)) ;
                break;
            case 1:
                scans[1] = (SamplesPortG[NSAMPLE] & _BV(PG5)) ;
                break;
            case 2:
                scans[2] = (SamplesPortG[NSAMPLE] & _BV(PG2)) ;
                break;
            case 3:
                scans[3] = (SamplesPortD[NSAMPLE] & _BV(PD6)) ;
                break;
            case 4:
                scans[4] = (SamplesPortG[NSAMPLE] & _BV(PG4)) ;
                break;
            case 5:
                scans[5] = (SamplesPortG[NSAMPLE] & _BV(PG1)) ;
                break;
            case 6:
                scans[6] = (SamplesPortD[NSAMPLE] & _BV(PD5)) ;
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

