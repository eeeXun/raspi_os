#include "shell.h"
#include "cpio.h"
#include "gic.h"
#include "mbox.h"
#include "program.h"
#include "reboot.h"
#include "register.h"
#include "string.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"

char* cpio_base;

#define MAX_ARGS 8
#define CMD_LEN 32
#define MSG_LEN 64
typedef struct {
    char cmd[CMD_LEN];
    char message[MSG_LEN];
    void (*exec_func)(int argc, char* argv[]);
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
    { .cmd = "timer-on",
        .message = "timer interrupt on",
        .exec_func = cmd_timer_on },
    { .cmd = "timer-off",
        .message = "timer interrupt off",
        .exec_func = cmd_timer_off },
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

void cmd_hello(int argc, char* argv[]) { uart_puts("Hello World!\n"); }

void cmd_ls(int argc, char* argv[])
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

void cmd_cat(int argc, char* argv[])
{
    unsigned int filesize;
    char* data;
    for (int i = 1; i < argc; i++) {
        if (!cpio_find(&filesize, argv[i], &data)) {
            uart_puts("cat: ");
            uart_puts(argv[i]);
            uart_puts(": No such file\n");
            continue;
        }
        for (unsigned int j = 0; j < filesize; j++)
            uart_put(data[j]);
    }
    uart_put('\n');
}

void cmd_run(int argc, char* argv[])
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

void cmd_timer_on(int argc, char* argv[])
{
    // [1] = 0, Not masked timer interrupt by IMASK bit
    // [0] = 1, enable timer
    write_reg(cntp_ctl_el0, 1);
    reset_timer(2); // expire in 2 seconds
    gic_enable(INTID_TIMER);
}

void cmd_timer_off(int argc, char* argv[])
{
    write_reg(cntp_ctl_el0, 0); // disable timer
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

void cmd_help(int argc, char* argv[])
{
    for (int i = 0; i < LEN(cmd_list); i++) {
        format_info_output(cmd_list[i].cmd);
        uart_puts(cmd_list[i].message);
        uart_puts("\n");
    }
}

void cmd_info_firmware_revision(int argc, char* argv[])
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

void cmd_info_board_model(int argc, char* argv[])
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

void cmd_info_board_revision(int argc, char* argv[])
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

void cmd_info_mac(int argc, char* argv[])
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

void cmd_info_board_serial(int argc, char* argv[])
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

void cmd_info_memory(int argc, char* argv[])
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

void cmd_info_all(int argc, char* argv[])
{
    cmd_info_firmware_revision(argc, argv);
    cmd_info_board_model(argc, argv);
    cmd_info_board_revision(argc, argv);
    cmd_info_mac(argc, argv);
    cmd_info_board_serial(argc, argv);
    cmd_info_memory(argc, argv);
}

void cmd_reboot(int argc, char* argv[])
{
    uart_puts("Rebooting...\n");
    reset(100);
}

void cmd_exec(char* buf, int cmd_len)
{
    int argc = 0;
    char* argv[MAX_ARGS];
    for (int i = 0; i < cmd_len;) {
        while (i < cmd_len && buf[i] == ' ') {
            buf[i] = '\0';
            i++;
        }
        if (i >= cmd_len)
            break;
        if (argc >= MAX_ARGS) {
            uart_puts("Too many args!\n");
            return;
        }
        argv[argc++] = &buf[i];
        while (i < cmd_len && buf[i] != ' ')
            i++;
    }
    if (argc == 0)
        return;
    for (int i = 0; i < LEN(cmd_list); i++) {
        if (strcmp(argv[0], cmd_list[i].cmd) == 0) {
            cmd_list[i].exec_func(argc, argv);
            return;
        }
    }
    cmd_not_found(argv[0]);
}

int cmd_read(char* buf)
{
    int idx = 0;
    while (1) {
        if (idx >= CLI_MAX_LEN) {
            uart_puts("\ncommand line too long\n");
            return -1;
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
            return idx;
        buf[idx] = c;
        idx++;
    }
}
