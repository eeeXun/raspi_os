all: bootloader.img

CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)

CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -nostdinc -nostdlib -nostartfiles -mstrict-align

boot.o: boot.s
	${CC} $(CFLAGS) -c boot.s -o boot.o

%.o: %.c
	${CC} $(CFLAGS) -c $< -o $@

bootloader.elf: boot.o $(OFILES) link.ld
	${LD} -nostdlib boot.o $(OFILES) -T link.ld -o bootloader.elf

bootloader.img: bootloader.elf
	${OBJCOPY} -O binary bootloader.elf bootloader.img

run: bootloader.img
	qemu-system-aarch64 -M raspi4b -kernel bootloader.img -serial null -serial pty -display none

clean:
	rm *.o *.elf *.img
