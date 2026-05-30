#include "mbox.h"
#include "gpio.h"
#include "mmio.h"
#include "utils.h"

// Reference https://github.com/raspberrypi/firmware/wiki/Mailboxes
enum {
    VIDEOCORE_MBOX = (PERIPHERAL_BASE + 0x0000B880),
    MBOX_READ = (VIDEOCORE_MBOX + 0x0),
    MBOX_POLL = (VIDEOCORE_MBOX + 0x10),
    MBOX_SENDER = (VIDEOCORE_MBOX + 0x14),
    MBOX_STATUS = (VIDEOCORE_MBOX + 0x18),
    MBOX_CONFIG = (VIDEOCORE_MBOX + 0x1C),
    MBOX_WRITE = (VIDEOCORE_MBOX + 0x20),
};

#define MBOX_FULL 0x80000000
#define MBOX_EMPTY 0x40000000
#define MBOX_CHANNEL 8

// Reference
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
#define MBOX_REQUEST_CODE 0x00000000
#define MBOX_TAG_REQUEST_CODE 0x00000000
#define MBOX_REQUEST_SUCCESSFUL 0x80000000
#define MBOX_END_TAG 0x00000000

// The buffer must be 16-byte aligned as only the upper 28 bits of the address
// can be passed via the mailbox
volatile unsigned int __attribute__((aligned(16))) mbox[8];

#define MBOX_SIZE (sizeof(mbox[0]) * LEN(mbox))

#define GET_FIRMWARE_REVISION 0x00000001
#define GET_BOARD_MODEL 0x00010001
#define GET_BOARD_REVISION 0x00010002
#define GET_BOARD_MAC 0x00010003
#define GET_BOARD_SERIAL 0x00010004
#define GET_ARM_MEMORY 0x00010005

unsigned int mbox_call()
{
    // 28-bit address (MSB) and 4-bit value (LSB)
    unsigned int r
        = ((unsigned int)((long)&mbox) & ~0xF) | (MBOX_CHANNEL & 0xF);

    // Wait until we can write
    while (mmio_read(MBOX_STATUS) & MBOX_FULL)
        ;

    // Write the address of our buffer to the mailbox with the channel appended
    mmio_write(MBOX_WRITE, r);

    while (1) {
        // Is there a reply?
        while (mmio_read(MBOX_STATUS) & MBOX_EMPTY)
            ;

        // Is it a reply to our message?
        if (r == mmio_read(MBOX_READ))
            return mbox[1] == MBOX_REQUEST_SUCCESSFUL; // Is it successful?
    }
    return 0;
}

int get_firmware_revision()
{
    mbox[0] = MBOX_SIZE; // buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    // tags begin
    mbox[2] = GET_FIRMWARE_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_END_TAG;

    return mbox_call();
}

int get_board_model()
{
    mbox[0] = MBOX_SIZE; // buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_MODEL; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_END_TAG;

    return mbox_call();
}

int get_board_revision()
{
    mbox[0] = MBOX_SIZE; // buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_END_TAG;

    return mbox_call();
}

int get_board_mac()
{
    mbox[0] = MBOX_SIZE; // buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_MAC; // tag identifier
    mbox[3] = 6; // maximum of request and response value buffer's length.
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_END_TAG;

    return mbox_call();
}

int get_board_serial()
{
    mbox[0] = MBOX_SIZE; // buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_SERIAL; // tag identifier
    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_END_TAG;

    return mbox_call();
}

int get_arm_memory()
{
    mbox[0] = MBOX_SIZE; // buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    // tags begin
    mbox[2] = GET_ARM_MEMORY; // tag identifier
    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_END_TAG;

    return mbox_call();
}
