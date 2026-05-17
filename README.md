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

Copy all the files in [firmware][firmware] to SD card

## Start

- Copy `kernel8.img` to SD card
- Connect with serial device
  `sudo minicom -b 115200 -D /dev/ttyUSB0`
