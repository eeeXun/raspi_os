# Lab 2

To put bootloader at 0x60000 and load `bootloader.img` instead of `kernel8.im`, put the following in `config.txt`

```
kernel_address=0x60000
kernel=bootloader.img
arm_64bit=1
```
