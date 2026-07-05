#include "cpio.h"
#include "string.h"
#include "uart.h"
#include "utils.h"

#define CPIO_MAGIC "070701"
#define CPIO_TRAILER "TRAILER!!!"

unsigned int hex2int(char* str)
{
    char c;
    unsigned int ret = 0;
    for (int i = 0; i < 8; i++) {
        ret <<= 4;
        c = str[i];
        if (c >= '0' && c <= '9')
            ret |= (c - '0');
        else
            ret |= ((c - 'A') + 10);
    }
    return ret;
}

cpio_newc_header* cpio_next(
    cpio_newc_header* header, unsigned int* filesize, char** name, char** data)
{
    if (strncmp(header->c_magic, CPIO_MAGIC, 6) != 0)
        return 0;

    *name = ((char*)header + sizeof(cpio_newc_header));
    if (strcmp(*name, CPIO_TRAILER) == 0)
        return 0;

    *filesize = hex2int(header->c_filesize);
    unsigned int namesize = hex2int(header->c_namesize);
    *data = (char*)header + align4(sizeof(cpio_newc_header) + namesize);
    return (cpio_newc_header*)(*data + align4(*filesize));
}
