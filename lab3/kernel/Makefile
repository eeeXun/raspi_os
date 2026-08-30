all: kernel8.img

CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

SFILES = $(wildcard *.s)
CFILES = $(wildcard *.c)
OFILES = $(SFILES:.s=.o) $(CFILES:.c=.o)

CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -nostdinc -nostdlib -nostartfiles -mstrict-align

%.o: %.s
	${CC} $(CFLAGS) -c $< -o $@

%.o: %.c
	${CC} $(CFLAGS) -c $< -o $@

kernel8.elf: $(OFILES) link.ld
	${LD} -nostdlib $(OFILES) -T link.ld -o kernel8.elf

kernel8.img: kernel8.elf
	${OBJCOPY} -O binary kernel8.elf kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi4b -kernel kernel8.img -initrd ../initramfs.cpio \
		-dtb ../../firmware/bcm2711-rpi-4-b.dtb -serial null -serial stdio -display none

clean:
	rm *.o *.elf *.img
