all: kernel8.img

CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -mstrict-align

boot.o: boot.s
	${CC} $(CFLAGS) -c boot.s -o boot.o

%.o: %.c
	${CC} $(CFLAGS) -c $< -o $@

kernel8.elf: boot.o $(OFILES) link.ld
	${LD} -nostdlib boot.o $(OFILES) -T link.ld -o kernel8.elf

kernel8.img: kernel8.elf
	${OBJCOPY} -O binary kernel8.elf kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi4b -kernel kernel8.img -serial null -serial stdio -display none

clean:
	rm *.o *.elf *.img
