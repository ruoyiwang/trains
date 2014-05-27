#
# Makefile for busy-wait IO library
#
XCC     = gcc
AS	= as
AR	= ar
CFLAGS  = -c -fPIC -Wall -I. -I./include -I./lib -I./kernel -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -N -Map kernel.map -T orex.ld -Llib -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all:  lib/libbwio.a kernel.s kernel.elf 

kernel.s: kernel/kernel.c kernel/kernel.h kernel/nameserver.h kernel/queue.h lib/bwio.h
	$(XCC) -S $(CFLAGS) kernel/kernel.c

kernel.o: kernel.s kernel/ker_ent_exit.asm
	$(AS) $(ASFLAGS) -o kernel.o kernel.s kernel/ker_ent_exit.asm

nameserver.s: kernel/nameserver.c kernel/nameserver.h
	$(XCC) -S $(CFLAGS) kernel/nameserver.c

nameserver.o: nameserver.s
	$(AS) $(ASFLAGS) -o nameserver.o nameserver.s

queue.s: kernel/queue.c kernel/queue.h
	$(XCC) -S $(CFLAGS) kernel/queue.c

queue.o: queue.s
	$(AS) $(ASFLAGS) -o queue.o queue.s

bwio.s: lib/bwio.c lib/bwio.h
	$(XCC) -S $(CFLAGS) lib/bwio.c

lib/libbwio.a: bwio.s
	$(AS) $(ASFLAGS) -o $@ bwio.s

kernel.elf: kernel.o nameserver.o queue.o
	$(LD) $(LDFLAGS) -o $@ kernel.o nameserver.o queue.o -lbwio -lgcc

clean:
	-rm -f *.s *.a *.o kernel.map
