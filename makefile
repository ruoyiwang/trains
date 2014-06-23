#
# Makefile for busy-wait IO library
#
XCC     = gcc
AS	= as
AR	= ar
CFLAGS  = -c -fPIC -Wall -I. -I./include -I./lib -I./kernel -I./tasks -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -N -Map kernel.map -T orex.ld -Llib -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all:  lib/libbwio.a lib/libutil.a kernel.s kernel.elf


kernel.s: kernel/kernel.c kernel/kernel.h kernel/nameserver.h tasks/clockserver.h tasks/comservers.h tasks/interface.h kernel/queue.h tasks/Tasks.h tasks/train.h lib/bwio.h
	$(XCC) -S $(CFLAGS) -O2 kernel/kernel.c

kernel.o: kernel.s kernel/ker_ent_exit.asm kernel/int_ker_ent_exit.asm
	$(AS) $(ASFLAGS) -o kernel.o kernel.s kernel/ker_ent_exit.asm kernel/int_ker_ent_exit.asm


nameserver.s: kernel/nameserver.c kernel/nameserver.h
	$(XCC) -S $(CFLAGS) kernel/nameserver.c

nameserver.o: nameserver.s
	$(AS) $(ASFLAGS) -o nameserver.o nameserver.s


clockserver.s: tasks/clockserver.c tasks/clockserver.h
	$(XCC) -S $(CFLAGS) tasks/clockserver.c

clockserver.o: clockserver.s
	$(AS) $(ASFLAGS) -o clockserver.o clockserver.s


comservers.s: tasks/comservers.c tasks/comservers.h
	$(XCC) -S $(CFLAGS) tasks/comservers.c

comservers.o: comservers.s
	$(AS) $(ASFLAGS) -o comservers.o comservers.s


interface.s: tasks/interface.c tasks/interface.h
	$(XCC) -S $(CFLAGS) tasks/interface.c

interface.o: interface.s
	$(AS) $(ASFLAGS) -o interface.o interface.s


train.s: tasks/train.c tasks/train.h
	$(XCC) -S $(CFLAGS) tasks/train.c

train.o: train.s
	$(AS) $(ASFLAGS) -o train.o train.s


queue.s: kernel/queue.c kernel/queue.h
	$(XCC) -S $(CFLAGS) kernel/queue.c

queue.o: queue.s
	$(AS) $(ASFLAGS) -o queue.o queue.s


Tasks.s: tasks/Tasks.c tasks/Tasks.h
	$(XCC) -S $(CFLAGS) tasks/Tasks.c

Tasks.o: Tasks.s lib/bwio.h
	$(AS) $(ASFLAGS) -o $@ Tasks.s


bwio.s: lib/bwio.c lib/bwio.h
	$(XCC) -S $(CFLAGS) -O2 lib/bwio.c

lib/libbwio.a: bwio.s
	$(AS) $(ASFLAGS) -o $@ bwio.s


util.s: lib/util.c lib/util.h
	$(XCC) -S $(CFLAGS) lib/util.c

lib/libutil.a: util.s
	$(AS) $(ASFLAGS) -o $@ util.s


kernel.elf: kernel.o nameserver.o clockserver.o comservers.o interface.o queue.o Tasks.o train.o
	$(LD) $(LDFLAGS) -o $@ kernel.o nameserver.o clockserver.o comservers.o interface.o queue.o Tasks.o train.o -lbwio -lutil -lgcc

clean:
	-rm -f *.s *.a *.o kernel.map
