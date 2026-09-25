#include "shell.h"
#include "uart.h"

int main()
{
    uart_init();
    int cmd_ret;
    char buf[CLI_MAX_LEN + 1] = {};
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
