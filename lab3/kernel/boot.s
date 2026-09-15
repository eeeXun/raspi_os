.section ".text.boot"

_start:
	// Set mpidr_el1, which could be the ID of processor, to x1
	mrs x1, mpidr_el1
	and x1, x1, #0x3
	cbz x1, from_el2_to_el1

lazy_loop:
	wfe
	b lazy_loop

from_el2_to_el1:
	// hcr_el2.RW = 1, so EL1 runs in AArch64 instead of AArch32
	mov x1, #(1 << 31)
	msr hcr_el2, x1

	// 0x3C5
	// [9:6], DAIF = 0b1111, all masked
	// M[3:0] = 0b0101 = EL1h (EL1 with sp_el1, sp becomes sp_el1)
	mov x1, #0x3C5
	msr spsr_el2, x1 // Saved Program Status Register, holds the saved process state

	// timer
	mrs x1, cnthctl_el2
	orr x1, x1, #0b11   // EL1PCEN (physical timer enable) | EL1PCTEN (physical counter enable)
	msr cnthctl_el2, x1

	// Where eret lands
	ldr x1, =init
	msr elr_el2, x1 // Exception Link Register, holds the address to return to

	eret

init:
	// Set exception vector table
	ldr x1, =exception_vector_table
	msr vbar_el1, x1

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
