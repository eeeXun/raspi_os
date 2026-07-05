#include "dtb.h"
#include "shell.h"
#include "uart.h"

int main(void* dtb_base)
{
    uart_init();
    fdt_tranverse(dtb_base, "linux,initrd-start", &cpio_base);
    int cmd_ret;
    char buf[CLI_MAX_LEN];
    while (1) {
        for (int i = 0; i < CLI_MAX_LEN; i++)
            buf[i] = '\0';
        uart_puts("# ");
        cmd_ret = cmd_read(buf);
        if (cmd_ret != 0)
            continue;
        cmd_exec(buf);
    }

    return 0;
}
