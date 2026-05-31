extern volatile unsigned int mbox[8];

// Reference
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
typedef enum MBOX_REQUEST_TAG {
    MBOX_TAG_FIRMWARE_REVISION = 0x00000001,
    MBOX_TAG_BOARD_MODEL = 0x00010001,
    MBOX_TAG_BOARD_REVISION = 0x00010002,
    MBOX_TAG_BOARD_MAC = 0x00010003,
    MBOX_TAG_BOARD_SERIAL = 0x00010004,
    MBOX_TAG_ARM_MEMORY = 0x00010005,
} MBOX_REQUEST_TAG;

int mbox_get_info(MBOX_REQUEST_TAG tag);
