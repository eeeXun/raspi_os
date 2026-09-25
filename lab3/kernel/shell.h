#define CLI_MAX_LEN 50

extern char* cpio_base;

void cmd_hello(int argc, char* argv[]);
void cmd_help(int argc, char* argv[]);
void cmd_ls(int argc, char* argv[]);
void cmd_cat(int argc, char* argv[]);
void cmd_run(int argc, char* argv[]);
void cmd_timer_on(int argc, char* argv[]);
void cmd_timer_off(int argc, char* argv[]);
void cmd_info_firmware_revision(int argc, char* argv[]);
void cmd_info_board_model(int argc, char* argv[]);
void cmd_info_board_revision(int argc, char* argv[]);
void cmd_info_mac(int argc, char* argv[]);
void cmd_info_board_serial(int argc, char* argv[]);
void cmd_info_memory(int argc, char* argv[]);
void cmd_info_all(int argc, char* argv[]);
void cmd_reboot(int argc, char* argv[]);

void cmd_exec(char* buf, int cmd_len);
int cmd_read(char* buf);
