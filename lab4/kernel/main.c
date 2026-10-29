#include "cpio.h"
#include "dtb.h"
#include "gic.h"
#include "memory.h"
#include "shell.h"
#include "timer.h"
#include "uart.h"

int main(void* dtb_base)
{
    uart_init();
    gic_init();
    enable_uart_interrupt();
    timer_init();
    asm volatile("msr DAIFClr, 0xF"); // Unmask all DAIF

    int fdt_prop_len;
    void* fdt_prop_addr;
    fdt_prop_addr
        = fdt_traverse(dtb_base, 0, "linux,initrd-start", &fdt_prop_len);
    if (fdt_prop_addr)
        cpio_base = (char*)fdt_read_cells(
            fdt_prop_addr, fdt_prop_len / 4 // 1 cell = 4 byte
        );
    buddy_init(FRAME_START, FRAME_END);
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
