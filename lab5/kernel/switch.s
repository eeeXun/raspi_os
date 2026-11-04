.section ".text"

.global enter_el0

enter_el0:
	msr elr_el1, x0 // Exception Link Register, user program address
	msr sp_el0, x1  // user stack pointer in EL0

	// 0x340
	// [9:6], DAIF = 0b1101, I bit is not masked
	// M[3:0] = 0b0000 = EL0
	mov x2, #0x340
	msr spsr_el1, x2
	eret

	.global switch_to

switch_to:
	stp x19, x20, [x0, 16 * 0]
	stp x21, x22, [x0, 16 * 1]
	stp x23, x24, [x0, 16 * 2]
	stp x25, x26, [x0, 16 * 3]
	stp x27, x28, [x0, 16 * 4]
	stp fp, lr, [x0, 16 * 5]
	mov x9, sp
	str x9, [x0, 16 * 6]

	ldp x19, x20, [x1, 16 * 0]
	ldp x21, x22, [x1, 16 * 1]
	ldp x23, x24, [x1, 16 * 2]
	ldp x25, x26, [x1, 16 * 3]
	ldp x27, x28, [x1, 16 * 4]
	ldp fp, lr, [x1, 16 * 5]
	ldr x9, [x1, 16 * 6]
	mov sp, x9
	msr tpidr_el1, x1
	ret

.global get_current_thread

get_current_thread:
	mrs x0, tpidr_el1
	ret
