# Raspberry Pi 4B bare metal programming

## Prerequisite

### SD card and filesystem format

- Create label type (or partition table type)
  - `sudo cfdisk /dev/sdc` if SD card is empty
  - `sudo cfdisk -z /dev/sdc` if SD card is not empty
- Choose `dos` label type
- Create a partition with `W95 FAT32 (LAB)` partition type
  - Partition type code `c`
- Format disk
  - `sudo mkfs.vfat -F 32 -n boot /dev/sdc1`

### Firmware

Copy all the files in [firmware](firmware) to SD card

### Config

Set clock rate in `config.txt`

```
core_freq_min=500
```

## Start

- Copy `kernel8.img` to SD card
- Connect with serial device
  `sudo minicom -b 115200 -D /dev/ttyUSB0`

## Reference

- [rpi4-osdev](https://github.com/sypstraw/rpi4-osdev)
- [OSC class](https://nycu-caslab.github.io/OSC2024/index.html)
- [peripherals](https://pip-assets.raspberrypi.com/categories/545-raspberry-pi-4-model-b/documents/RP-008248-DS-1-bcm2711-peripherals.pdf?disposition=inline)
- [Instructions Set](https://developer.arm.com/documentation/ddi0602/2026-03/Base-Instructions?lang=en)
- [Mailbox](https://github.com/raspberrypi/firmware/wiki/Mailboxes)
- [Mailbox property](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)
- [Devicetree Blob format](https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html)
