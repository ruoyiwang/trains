#
# Makefile for busy-wait IO library
#
XCC     = gcc
AS	= as
AR	= ar
CFLAGS  = -c -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -N -Map kernel.map -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all:  kernel.s kernel.elf

kernel.s: kernel.c kernel.h nameserver.h
	$(XCC) -S $(CFLAGS) kernel.c

kernel.o: kernel.s ker_ent_exit.asm
	$(AS) $(ASFLAGS) -o kernel.o kernel.s ker_ent_exit.asm

nameserver.s: nameserver.c nameserver.h
	$(XCC) -S $(CFLAGS) nameserver.c

nameserver.o: nameserver.s
	$(AS) $(ASFLAGS) -o nameserver.o nameserver.s


kernel.elf: kernel.o nameserver.o
	$(LD) $(LDFLAGS) -o $@ kernel.o nameserver.o -lbwio -lgcc

clean:
	-rm -f *.s *.a *.o kernel.map
