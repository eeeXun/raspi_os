// See
// https://pip-assets.raspberrypi.com/categories/545-raspberry-pi-4-model-b/documents/RP-008248-DS-1-bcm2711-peripherals.pdf
#define INTID_TIMER 30 // Core n PNS timer IRQ

void gic_init();
void gic_enable(unsigned int intid);
unsigned int gic_ack();
void gic_eoi(unsigned int intid);
