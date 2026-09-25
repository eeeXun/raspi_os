#include "dtb.h"
#include "gic.h"
#include "shell.h"
#include "uart.h"

int main(void* dtb_base)
{
    uart_init();
    gic_init();
    enable_uart_interrupt();
    asm volatile("msr DAIFClr, 0xF"); // Unmask all DAIF

    fdt_tranverse(dtb_base, "linux,initrd-start", &cpio_base);
    int cmd_len;
    char buf[CLI_MAX_LEN + 1] = {};
    while (1) {
        for (int i = 0; i < CLI_MAX_LEN; i++)
            buf[i] = '\0';
        uart_puts("# ");
        cmd_len = cmd_read(buf);
        if (cmd_len < 0)
            continue;
        cmd_exec(buf, cmd_len);
    }

    return 0;
}
