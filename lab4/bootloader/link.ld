SECTIONS
{
    . = 0x60000;
    _kernel_start = 0x80000;
    .text : { *(.text.boot) *(.text) }
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
