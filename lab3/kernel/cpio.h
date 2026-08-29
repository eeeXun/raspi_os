// Defined in shell.c, filled in from the dtb at boot
extern char* cpio_base;

typedef struct {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
} cpio_newc_header;

cpio_newc_header* cpio_next(
    cpio_newc_header* header, unsigned int* filesize, char** name, char** data);

int cpio_find(unsigned int* filesize, char* name, char** data);
