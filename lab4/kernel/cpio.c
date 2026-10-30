#include "cpio.h"
#include "dtb.h"
#include "string.h"
#include "utils.h"

#define CPIO_MAGIC "070701"
#define CPIO_TRAILER "TRAILER!!!"

char* cpio_base;

void initrd_init(void* dtb_base)
{
    int prop_len;
    void* prop_addr;
    prop_addr = fdt_traverse(dtb_base, 0, "linux,initrd-start", &prop_len);
    if (prop_addr)
        cpio_base
            = (char*)fdt_read_cells(prop_addr, prop_len / 4 // 1 cell = 4 byte
            );
}

long long hex2longlong(char* str)
{
    char c;
    long long ret = 0;
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
    cpio_newc_header* header, long long* filesize, char** name, char** data)
{
    if (strncmp(header->c_magic, CPIO_MAGIC, 6) != 0)
        return 0;

    *name = ((char*)header + sizeof(cpio_newc_header));
    if (strcmp(*name, CPIO_TRAILER) == 0)
        return 0;

    *filesize = hex2longlong(header->c_filesize);
    long long namesize = hex2longlong(header->c_namesize);
    *data = (char*)header + align4(sizeof(cpio_newc_header) + namesize);
    return (cpio_newc_header*)(*data + align4(*filesize));
}

int cpio_find(long long* filesize, char* name, char** data)
{
    cpio_newc_header* header = (cpio_newc_header*)cpio_base;
    char* entry_name;
    while ((header = cpio_next(header, filesize, &entry_name, data)) != 0) {
        if (strcmp(name, entry_name) == 0)
            return 1;
    }
    return 0;
}
