#include "shell.h"
#include "cpio.h"
#include "mbox.h"
#include "program.h"
#include "reboot.h"
#include "string.h"
#include "uart.h"
#include "utils.h"

char* cpio_base;

#define CMD_LEN 32
#define MSG_LEN 64
typedef struct {
    char cmd[CMD_LEN];
    char message[MSG_LEN];
    void (*exec_func)();
} CMDS;

CMDS cmd_list[] = {
    { .cmd = "help", .message = "print this help menu", .exec_func = cmd_help },
    { .cmd = "hello", .message = "print Hello World!", .exec_func = cmd_hello },
    { .cmd = "ls",
        .message = "list files in the initramfs",
        .exec_func = cmd_ls },
    { .cmd = "cat",
        .message = "print a file from the initramfs",
        .exec_func = cmd_cat },
    { .cmd = "run",
        .message = "run user program from the initramfs",
        .exec_func = cmd_run },
    { .cmd = "reboot",
        .message = "reboot raspberry pi",
        .exec_func = cmd_reboot },
    { .cmd = "info-firmware-revision",
        .message = "print firmware revision",
        .exec_func = cmd_info_firmware_revision },
    { .cmd = "info-board-model",
        .message = "print board model",
        .exec_func = cmd_info_board_model },
    { .cmd = "info-board-revision",
        .message = "print board revision",
        .exec_func = cmd_info_board_revision },
    { .cmd = "info-mac",
        .message = "print mac address",
        .exec_func = cmd_info_mac },
    { .cmd = "info-board-serial",
        .message = "print board serial",
        .exec_func = cmd_info_board_serial },
    { .cmd = "info-memory",
        .message = "print arm memory",
        .exec_func = cmd_info_memory },
    { .cmd = "info-all",
        .message = "print all hardware info",
        .exec_func = cmd_info_all },
};

void cmd_not_found(char* buf)
{
    uart_puts("command not found: ");
    uart_puts(buf);
    uart_puts("\n");
}

void cmd_hello() { uart_puts("Hello World!\n"); }

void cmd_ls()
{
    cpio_newc_header* header = (cpio_newc_header*)cpio_base;
    unsigned int filesize;
    char *name, *data;
    while ((header = cpio_next(header, &filesize, &name, &data)) != 0) {
        if (strcmp(name, ".") == 0)
            continue;
        uart_puts(name);
        uart_put('\n');
    }
}

void cmd_cat()
{
    unsigned int filesize;
    char* data;
    char input_name[CLI_MAX_LEN] = {};
    uart_puts("Filename: ");
    cmd_read(input_name);
    if (!cpio_find(&filesize, input_name, &data)) {
        uart_puts("cat: ");
        uart_puts(input_name);
        uart_puts(": No such file\n");
        return;
    }
    for (unsigned int i = 0; i < filesize; i++)
        uart_put(data[i]);
    uart_put('\n');
}

void cmd_run()
{
    unsigned int filesize;
    char* data;
    char input_name[CLI_MAX_LEN] = {};
    uart_puts("Filename: ");
    cmd_read(input_name);
    if (!cpio_find(&filesize, input_name, &data)) {
        uart_puts("run: ");
        uart_puts(input_name);
        uart_puts(": No such file\n");
        return;
    }
    run_user_program(data, filesize);
}

void format_info_output(char* s)
{
    int cmd_len = strlen(s);
    uart_puts(s);
    uart_put(':');
    if (cmd_len < 7)
        uart_put('\t');
    if (cmd_len < 15)
        uart_put('\t');
    if (cmd_len < 23)
        uart_put('\t');
}

void cmd_help()
{
    for (int i = 0; i < LEN(cmd_list); i++) {
        format_info_output(cmd_list[i].cmd);
        uart_puts(cmd_list[i].message);
        uart_puts("\n");
    }
}

void cmd_info_firmware_revision()
{
    if (!mbox_get_info(MBOX_TAG_FIRMWARE_REVISION)) {
        uart_puts("Get Firmware Revision Failed!\n");
        return;
    }
    format_info_output("Firmware Revision");
    uart_puts("0x");
    uart_put_hex(mbox[5]);
    uart_put('\n');
}

void cmd_info_board_model()
{
    if (!mbox_get_info(MBOX_TAG_BOARD_MODEL)) {
        uart_puts("Get Board Model Failed!\n");
        return;
    }
    format_info_output("Board Model");
    uart_puts("0x");
    uart_put_hex(mbox[5]);
    uart_put('\n');
}

void cmd_info_board_revision()
{
    if (!mbox_get_info(MBOX_TAG_BOARD_REVISION)) {
        uart_puts("Get Board Revision Failed!\n");
        return;
    }
    format_info_output("Board Revision");
    uart_puts("0x");
    uart_put_hex(mbox[5]);
    uart_put('\n');
}

void cmd_info_mac()
{
    if (!mbox_get_info(MBOX_TAG_BOARD_MAC)) {
        uart_puts("Get Mac Address Failed!\n");
        return;
    }
    format_info_output("Mac Address");
    unsigned char* mac = (unsigned char*)&mbox[5];
    uart_put_hex(mac[0]);
    for (int i = 1; i < 6; i++) {
        uart_put(':');
        uart_put_hex(mac[i]);
    }
    uart_put('\n');
}

void cmd_info_board_serial()
{
    if (!mbox_get_info(MBOX_TAG_BOARD_SERIAL)) {
        uart_puts("Get Board serial Failed\n");
        return;
    }
    format_info_output("Board Serial");
    uart_puts("0x");
    // little endian
    uart_put_hex(((unsigned long long)mbox[6] << 32) | mbox[5]);
    uart_put('\n');
}

void cmd_info_memory()
{
    if (!mbox_get_info(MBOX_TAG_ARM_MEMORY)) {
        uart_puts("Get Meomry Failed\n");
        return;
    }
    format_info_output("Memory Base Address");
    uart_puts("0x");
    uart_put_hex(mbox[5]);
    uart_put('\n');
    format_info_output("Memory Size");
    uart_puts("0x");
    uart_put_hex(mbox[6]);
    uart_put('\n');
}

void cmd_info_all()
{
    cmd_info_firmware_revision();
    cmd_info_board_model();
    cmd_info_board_revision();
    cmd_info_mac();
    cmd_info_board_serial();
    cmd_info_memory();
}

void cmd_reboot()
{
    uart_puts("Rebooting...\n");
    reset(100);
}

void cmd_exec(char* buf)
{
    if (*buf == '\0')
        return;
    for (int i = 0; i < LEN(cmd_list); i++) {
        if (strcmp(buf, cmd_list[i].cmd) == 0) {
            cmd_list[i].exec_func();
            return;
        }
    }
    cmd_not_found(buf);
}

int cmd_read(char* buf)
{
    int idx = 0;
    while (1) {
        if (idx >= CLI_MAX_LEN) {
            uart_puts("\ncommand line to long\n");
            return 1;
        }
        char c = uart_getc();
        if (c == '\b') { // backspace
            if (idx > 0) {
                uart_put(c);
                buf[--idx] = '\0';
            }
            continue;
        }
        uart_put(c);
        if (c == '\n')
            return 0;
        buf[idx] = c;
        idx++;
    }
}
