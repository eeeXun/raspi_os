# Lab 3

To put bootloader at 0x60000 and load `bootloader.img` instead of `kernel8.img`, put the following in `config.txt`

```
kernel_address=0x60000
kernel=bootloader.img
arm_64bit=1
```

Copy `initramfs.cpio` to SD card, and add loading address in `config.txt`

```
initramfs initramfs.cpio 0x20000000
```

To modify the cpio content, run

```
(cd rootfs && find . | cpio -o -H newc > ../initramfs.cpio)
```

Modify user program, run the following command. It will update `initramfs.cpio`

```
(cd user && make install)
```

After booting from `bootloader.img`, run the `boot` command. And send the kernel image with

```
sudo python send_img.py --file kernel/kernel8.img --serial /dev/ttyUSB0
```
