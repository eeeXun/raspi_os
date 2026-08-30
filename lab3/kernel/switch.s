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
