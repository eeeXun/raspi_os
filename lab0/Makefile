all: kernel8.img

a.o: a.s
	aarch64-linux-gnu-gcc -c a.s

kernel8.elf: a.o link.ld
	aarch64-linux-gnu-ld -T link.ld -o kernel8.elf a.o

kernel8.img: kernel8.elf
	aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm

clean:
	rm *.o *.elf *.img
