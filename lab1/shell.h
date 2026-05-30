#define CLI_MAX_LEN 50

#define CMD_LEN 32
#define MSG_LEN 64
typedef struct CMDS {
    char cmd[CMD_LEN];
    char message[MSG_LEN];
    void (*exec_func)();
} CMDS;

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
