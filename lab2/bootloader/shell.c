#include "shell.h"
#include "reboot.h"
#include "string.h"
#include "uart.h"
#include "utils.h"

extern char* _kernel_start;

#define CMD_LEN 32
#define MSG_LEN 64
#define BOOT_CHUNK                                                             \
    120 // bytes received per ACK; must match CHUNK in send_img.py
typedef struct CMDS {
    char cmd[CMD_LEN];
    char message[MSG_LEN];
    void (*exec_func)();
} CMDS;

struct CMDS cmd_list[] = {
    { .cmd = "help", .message = "print this help menu", .exec_func = cmd_help },
    { .cmd = "hello", .message = "print Hello World!", .exec_func = cmd_hello },
    { .cmd = "boot",
        .message = "boot an img from serial port",
        .exec_func = cmd_boot },
    { .cmd = "reboot",
        .message = "reboot raspberry pi",
        .exec_func = cmd_reboot },
};

void cmd_not_found(char* buf)
{
    uart_puts("command not found: ");
    uart_puts(buf);
    uart_puts("\n");
}

void cmd_hello() { uart_puts("Hello World!\n"); }

void format_output(char* s)
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
        format_output(cmd_list[i].cmd);
        uart_puts(cmd_list[i].message);
        uart_puts("\n");
    }
}

void cmd_boot()
{
    unsigned int size = 0;
    uart_puts("Getting boot size\n");
    for (int i = 0; i < 4; i++)
        size |= (uart_getb() << (i << 3)); // (i << 3) = (i * 8)
    uart_puts("Booting size: ");
    uart_put_dec(size);
    uart_put('\n');

    char* kernel_entry = (char*)(&_kernel_start);
    for (int i = 0; i < size; i++)
        kernel_entry[i] = uart_getb();
    uart_puts("Load img successfully\n");
    uart_puts("Start booting\n");

    ((void (*)())kernel_entry)();
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
