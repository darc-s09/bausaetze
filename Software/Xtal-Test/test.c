/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 */

#include <avr/io.h>
#include <avr/fuse.h>

FUSES =
{
  .low = (FUSE_SUT0 & FUSE_SUT0),  // low-power xtal, fast rising power
  .high = (FUSE_BOOTSZ0 & FUSE_BOOTSZ1 & FUSE_EESAVE & FUSE_SPIEN),
  .extended = EFUSE_DEFAULT,
};

int
main(void)
{
  DDRB = _BV(PB3); // OC2A = PB3 => JP1 / pin 4 as output

  TCCR2A = _BV(COM2A0) | // toggle OC2A on compare match
  _BV(WGM21); // CTC mode, OCR2A = top
  OCR2A = 0; // maximal frequency
  TCCR2B = _BV(CS20); // maximal frequency

  for (;;)
    {}
}
