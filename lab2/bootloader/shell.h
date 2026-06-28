#define CLI_MAX_LEN 50

void cmd_hello();
void cmd_help();
void cmd_boot();
void cmd_reboot();

void cmd_exec(char* buf);
int cmd_read(char* buf);
