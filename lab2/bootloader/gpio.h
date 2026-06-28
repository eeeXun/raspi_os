// From https://pip.raspberrypi.com/documents/RP-008248-DS
#define PERIPHERAL_BASE 0xFE000000

enum {
    GPIO_BASE = PERIPHERAL_BASE + 0x200000,
    GPFSEL1 = GPIO_BASE + 0x4,
    GPIO_PUP_PDN_CNTRL_REG0 = GPIO_BASE + 0xE4,
};
