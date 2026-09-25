// ┌──────────────────────┐
// │   GICD Distributor   │
// └─┬─────┬─────┬─────┬──┘
//   ↓     ↓     ↓     ↓
// ┌────┐┌────┐┌────┐┌────┐
// │GICC││GICC││GICC││GICC│
// └─┬──┘└─┬──┘└─┬──┘└─┬──┘
//   ↓     ↓     ↓     ↓
// core0 core1 core2 core3

#include "mmio.h"

#define GIC_BASE 0xFF840000
#define GICD_BASE (GIC_BASE + 0x1000)
#define GICC_BASE (GIC_BASE + 0x2000)

enum {
    GICD_CTLR = GICD_BASE, // Distributor Control Register
    GICD_ISENABLER = GICD_BASE + 0x100, // Interrupt Set-Enable Registers
    GICD_ITARGETSR = GICD_BASE + 0x800, // Interrupt Processor Targets Registers
    GICC_CTLR = GICC_BASE, // CPU Interface Control Register
    GICC_PMR = GICC_BASE + 0x4, // CPU Interface Priority Mask Register
    GICC_IAR = GICC_BASE + 0xC, // CPU Interface Interrupt Acknowledge Register
    GICC_EOIR = GICC_BASE + 0x10 // CPU Interface End Of Interrupt Register
};

void gic_init()
{
    mmio_write(GICD_CTLR, 1); // Enable GICD_CTLR
    mmio_write(GICC_CTLR, 1); // Enable GICC_CTLR
    mmio_write(GICC_PMR, 0xFF); // Enable all priority
}

// Reference
// Chap. 6.3 of
// https://pip-assets.raspberrypi.com/categories/545-raspberry-pi-4-model-b/documents/RP-008248-DS-1-bcm2711-peripherals.pdf,
// https://support.arm.com/documentation/ddi0601/2026-06/External-Registers/GICD-ITARGETSR-n---Interrupt-Processor-Targets-Registers?lang=en
// and
// https://support.arm.com/documentation/ddi0601/2026-06/External-Registers/GICD-ISENABLER-n---Interrupt-Set-Enable-Registers?lang=en
void gic_enable(unsigned int intid)
{
    // Shared Peripheral Interrupt, (SPI, intid >= 32)
    // Need to assign to a target core
    if (intid >= 32) {
        // Set intid-th byte start from GICD_ITARGETSR to 0b00000001, which
        // means assign to CPU0
        //
        // n = intid / 4
        // GICD_ITARGETSR[n] = GICD_ITARGETSR + 4 * n
        long reg = GICD_ITARGETSR + (intid & ~0b11);
        unsigned int value = mmio_read(reg);
        int shift = (intid & 0b11) << 3; // (intid % 4) * 8
        value &= ~(0xFF << shift); // clear shift byte
        value |= (1 << shift);
        mmio_write(reg, value);
    }
    // n = intid / 32
    // ISENABLER[n] = GICD_ISENABLER + 4 * n
    long reg = GICD_ISENABLER + ((intid >> 5) << 2);
    // set bit (intid % 32) in ISENABLER[n]
    mmio_write(reg, 1 << (intid & 31));
}

unsigned int gic_ack()
{
    // Only bit [9:0] are intid
    return mmio_read(GICC_IAR) & 0x3FF;
}

// GIC end of interrupt
void gic_eoi(unsigned int intid) { mmio_write(GICC_EOIR, intid); }
