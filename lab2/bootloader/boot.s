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
	adr x1, _start       // x1 = where we actually loaded (PC-relative)
	ldr x2, =_start      // x2 = where we want to run (absolute, = 0x60000)
	ldr x3, =__bss_start // copy text+rodata+data (everything before bss)
	sub x3, x3, x2       // x3 = byte count

relocate:
	// relocate is for cmd_list[i].exec_func() in shell.c
	// cmd_list[i].exec_func() is called with absolute address
	// We set location counter to 0x60000 in linker script
	// The cmd_list[i].exec_func() should located .data from 0x60000
	// But qemu always load into 0x80000
	// So the .data is not located from 0x60000
	// And it can't find the cmd_list[i].exec_func()
	ldr  x4, [x1], #8
	str  x4, [x2], #8
	sub  x3, x3, #8
	cbnz x3, relocate

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
