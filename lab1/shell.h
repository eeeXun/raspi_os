#define CLI_MAX_LEN 50

void cmd_hello();
void cmd_help();
void cmd_info_firmware_revision();
void cmd_info_board_model();
void cmd_info_board_revision();
void cmd_info_mac();
void cmd_info_board_serial();
void cmd_info_memory();
void cmd_info_all();
void cmd_reboot();

void cmd_exec(char* buf);
int cmd_read(char* buf);
