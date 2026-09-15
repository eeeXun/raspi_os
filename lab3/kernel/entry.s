// save general registers to stack
// stp: Store pair of registers
.macro save_all
sub    sp, sp, 32 * 8
stp    x0, x1, [sp, 16 * 0]
stp    x2, x3, [sp, 16 * 1]
stp    x4, x5, [sp, 16 * 2]
stp    x6, x7, [sp, 16 * 3]
stp    x8, x9, [sp, 16 * 4]
stp    x10, x11, [sp, 16 * 5]
stp    x12, x13, [sp, 16 * 6]
stp    x14, x15, [sp, 16 * 7]
stp    x16, x17, [sp, 16 * 8]
stp    x18, x19, [sp, 16 * 9]
stp    x20, x21, [sp, 16 * 10]
stp    x22, x23, [sp, 16 * 11]
stp    x24, x25, [sp, 16 * 12]
stp    x26, x27, [sp, 16 * 13]
stp    x28, x29, [sp, 16 * 14]
str    x30, [sp, 16 * 15]
.endm

// load general registers from stack
// ldp: Load pair of registers
.macro load_all
ldp    x0, x1, [sp, 16 * 0]
ldp    x2, x3, [sp, 16 * 1]
ldp    x4, x5, [sp, 16 * 2]
ldp    x6, x7, [sp, 16 * 3]
ldp    x8, x9, [sp, 16 * 4]
ldp    x10, x11, [sp, 16 * 5]
ldp    x12, x13, [sp, 16 * 6]
ldp    x14, x15, [sp, 16 * 7]
ldp    x16, x17, [sp, 16 * 8]
ldp    x18, x19, [sp, 16 * 9]
ldp    x20, x21, [sp, 16 * 10]
ldp    x22, x23, [sp, 16 * 11]
ldp    x24, x25, [sp, 16 * 12]
ldp    x26, x27, [sp, 16 * 13]
ldp    x28, x29, [sp, 16 * 14]
ldr    x30, [sp, 16 * 15]
add    sp, sp, 32 * 8
.endm

.section ".text.vector"

// vector table should be aligned to 2^11 = 0x800
.align  11
.global exception_vector_table

exception_vector_table:
	// entry size is 2^7 = 0x80
	// branch to a handler function.
	// Current EL with sp_el0 (EL1t)
	b      exception_entry // 0x000 synchronous
	.align 7
	b      irq_entry // 0x080 IRQ
	.align 7
	b      exception_entry // 0x100 FIQ
	.align 7
	b      exception_entry // 0x180 SError
	.align 7

	// Current EL with sp_el1 (EL1h)
	b      exception_entry // 0x200 synchronous
	.align 7
	b      irq_entry // 0x280 IRQ
	.align 7
	b      exception_entry // 0x300 FIQ
	.align 7
	b      exception_entry // 0x380 SError
	.align 7

	// Lower EL running AArch64, i.e. the user program at EL0
	b      exception_entry // 0x400 synchronous (svc, user fault)
	.align 7
	b      irq_entry // 0x480 IRQ (timer, uart)
	.align 7
	b      exception_entry // 0x500 FIQ
	.align 7
	b      exception_entry // 0x580 SError
	.align 7

	// Lower EL running AArch32. We never run 32 bit code
	b      exception_entry // 0x600 synchronous
	.align 7
	b      irq_entry // 0x680 IRQ
	.align 7
	b      exception_entry // 0x700 FIQ
	.align 7
	b      exception_entry // 0x780 SError
	.align 7

	.section ".text"

exception_entry:
	save_all
	bl exception_handler
	load_all
	eret

irq_entry:
	save_all
	bl irq_handler
	load_all
	eret

	// vim:ft=asm
