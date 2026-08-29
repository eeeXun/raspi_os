.section ".text.boot"

_start:
	// Set mpidr_el1, which could be the ID of processor, to x1
	mrs x1, mpidr_el1
	and x1, x1, #0x3
	cbz x1, init

lazy_loop:
	wfe
	b lazy_loop

init:
	// Set stack pointer to start, then it will push forward from start
	ldr x1, =_start
	mov sp, x1

	ldr x1, =__bss_start
	ldr w2, =__bss_size

init_bss:
	// Clean BSS section
	cbz  w2, run_main
	str  xzr, [x1], #8 // Store zero register to bss, then move x1 to next 8 byte
	sub  w2, w2, #1
	cbnz w2, init_bss

run_main:
	b main
