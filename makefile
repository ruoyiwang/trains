#
# Makefile for busy-wait IO library
#
XCC     = gcc
AS	= as
AR	= ar
CFLAGS  = -c -fPIC -Wall -I. -I./include -I./lib -I./kernel -I./tasks -I./track -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -N -Map kernel.map -T orex.ld -Llib -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all:  kernel.s kernel.elf


kernel.s: kernel/kernel.c kernel/kernel.h kernel/nameserver.h tasks/clockserver.h tasks/sensors.h tasks/commandcenter.h tasks/comservers.h tasks/interface.h kernel/queue.h tasks/Tasks.h tasks/train.h lib/bwio.h
	$(XCC) -S $(CFLAGS) kernel/kernel.c

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


sensors.s: tasks/sensors.c tasks/sensors.h
	$(XCC) -S $(CFLAGS) tasks/sensors.c

sensors.o: sensors.s
	$(AS) $(ASFLAGS) -o sensors.o sensors.s


commandcenter.s: tasks/commandcenter.c tasks/commandcenter.h
	$(XCC) -S $(CFLAGS) tasks/commandcenter.c

commandcenter.o: commandcenter.s
	$(AS) $(ASFLAGS) -o commandcenter.o commandcenter.s


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


track.s: tasks/track.c tasks/track.h
	$(XCC) -S $(CFLAGS) tasks/track.c

track.o: track.s
	$(AS) $(ASFLAGS) -o track.o track.s


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

bwio.o: bwio.s
	$(AS) $(ASFLAGS) -o $@ bwio.s


util.s: lib/util.c lib/util.h
	$(XCC) -S $(CFLAGS) -O2 lib/util.c

util.o: util.s
	$(AS) $(ASFLAGS) -o $@ util.s


posintlist.s: lib/posintlist.c lib/posintlist.h
	$(XCC) -S $(CFLAGS) -O2 lib/posintlist.c

posintlist.o: posintlist.s
	$(AS) $(ASFLAGS) -o $@ posintlist.s


debug.s: kernel/debug.c kernel/debug.h
	$(XCC) -S $(CFLAGS) kernel/debug.c

debug.o: debug.s
	$(AS) $(ASFLAGS) -o debug.o debug.s


track_data.s: track/track_data.c track/track_data.h track/track_node.h
	$(XCC) -S $(CFLAGS) track/track_data.c

track_data.o: track_data.s
	$(AS) $(ASFLAGS) -o track_data.o track_data.s


trainspeed.s: track/trainspeed.c track/trainspeed.h
	$(XCC) -S $(CFLAGS) track/trainspeed.c

trainspeed.o: trainspeed.s
	$(AS) $(ASFLAGS) -o trainspeed.o trainspeed.s


kernel.elf: kernel.o nameserver.o clockserver.o sensors.o commandcenter.o comservers.o interface.o queue.o Tasks.o train.o track.o bwio.o util.o posintlist.o debug.o track_data.o trainspeed.o
	$(LD) $(LDFLAGS) -o $@ kernel.o nameserver.o clockserver.o sensors.o commandcenter.o comservers.o interface.o queue.o Tasks.o train.o track.o bwio.o util.o debug.o posintlist.o track_data.o trainspeed.o -lgcc

clean:
	-rm -f *.s *.a *.o kernel.map
