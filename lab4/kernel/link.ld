SECTIONS
{
    /* Set location counter to 0x80000 */
    /* We can change it to any location, but this is better for debugging */

    /* objcopy elf file to img file will set file offset to 0 */
    /* When booting Raspberry Pi, it load the img file from offset 0 to 0x80000 */
    /* And run the first instruction at 0x80000 */
    . = 0x80000;
    .text : { *(.text.boot) *(.text.vector) *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : {
        /* Set location counter aligned to next 16 bit */
        /* Equivalent to (. + 16 - 1) & ~(16 - 1) */
        . = ALIGN(16);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
}
/* We move and clean bss section with 8 bytes */
__bss_size = (__bss_end - __bss_start) >> 3;
