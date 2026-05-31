#include "mbox.h"
#include "gpio.h"
#include "mmio.h"
#include "utils.h"

#define MBOX_FULL 0x80000000
#define MBOX_EMPTY 0x40000000
#define MBOX_CHANNEL 8

#define MBOX_REQUEST_CODE 0x00000000
#define MBOX_TAG_REQUEST_CODE 0x00000000
#define MBOX_REQUEST_SUCCESSFUL 0x80000000
#define MBOX_END_TAG 0x00000000

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

typedef struct MBOX_REQUEST {
    MBOX_REQUEST_TAG tag;
    int size;
} MBOX_REQUEST;

struct MBOX_REQUEST mbox_requests[] = {
    { .tag = MBOX_TAG_FIRMWARE_REVISION, .size = 4 },
    { .tag = MBOX_TAG_BOARD_MODEL, .size = 4 },
    { .tag = MBOX_TAG_BOARD_REVISION, .size = 4 },
    { .tag = MBOX_TAG_BOARD_MAC, .size = 6 },
    { .tag = MBOX_TAG_BOARD_SERIAL, .size = 8 },
    { .tag = MBOX_TAG_ARM_MEMORY, .size = 8 },
};

// The buffer must be 16-byte aligned as only the upper 28 bits of the address
// can be passed via the mailbox
volatile unsigned int __attribute__((aligned(16))) mbox[8];

int mbox_call()
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

int mbox_get_info(MBOX_REQUEST_TAG tag)
{
    mbox[0] = sizeof(mbox[0]) * LEN(mbox); // buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    for (int i = 0; i < LEN(mbox_requests); i++) {
        if (tag == mbox_requests[i].tag) {
            mbox[2] = mbox_requests[i].tag; // tag identifier
            mbox[3] = mbox_requests[i].size; // maximum of request and response
                                             // value buffer's length.
            break;
        }
    }
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_END_TAG;

    return mbox_call();
}
