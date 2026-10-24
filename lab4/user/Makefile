all: user.img

CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -nostdinc -mstrict-align

user.o: user.s
	${CC} $(CFLAGS) -c user.s -o user.o

user.elf: user.o link.ld
	${LD} -nostdlib user.o -T link.ld -o user.elf

user.img: user.elf
	${OBJCOPY} -O binary user.elf user.img

install: user.img
	cp user.img ../rootfs/user.img
	cd ../rootfs && find . | cpio -o -H newc > ../initramfs.cpio

clean:
	rm -f *.o *.elf *.img
