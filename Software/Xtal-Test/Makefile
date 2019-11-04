#
# $Id$
#
CC= avr-gcc
MCU=atmega328p

AVRVERIFY?=no

CFLAGS=  -Os -g3 -Wall -Wextra -mmcu=$(MCU)

.SUFFIXES: .s .bin .out .hex .srec


.c.s:
	$(CC) $(CFLAGS) -S $<

.S.o:
	$(CC) $(ASFLAGS) -c $<

all:	test.elf

OBJS=test.o
test.elf: $(OBJS)
	$(CC) -o test.elf $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS)

AVRDUDE_PROGRAMMER=avrisp2

program: test.elf
	avrdude -c ${AVRDUDE_PROGRAMMER} -p ${MCU} -U test.elf -U hfuse:w:test.elf -U lfuse:w:test.elf

clean:
	rm -f *~ *.elf *.bin *.hex *.srec *.s *.o *.pdf *.core
