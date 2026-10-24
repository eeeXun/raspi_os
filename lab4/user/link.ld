/* The kernel copies the image here before eret, see USER_PROGRAM_BASE */
SECTIONS
{
    . = 0x200000;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
