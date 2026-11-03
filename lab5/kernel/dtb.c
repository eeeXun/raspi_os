#include "string.h"
#include "utils.h"

#define FDT_MAGIC 0XD00DFEED
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

typedef struct {
    unsigned int magic;
    unsigned int totalsize;
    unsigned int off_dt_struct;
    unsigned int off_dt_strings;
    unsigned int off_mem_rsvmap;
    unsigned int version;
    unsigned int last_comp_version;
    unsigned int boot_cpuid_phys;
    unsigned int size_dt_strings;
    unsigned int size_dt_struct;
} fdt_header;

typedef struct {
    unsigned int len;
    unsigned int nameoff;
} fdt_prop;

unsigned int be2le(unsigned int num)
{
    return (num >> 24) | ((num >> 8) & 0xFF00) | ((num << 8) & 0xFF0000)
        | (num << 24);
}

void* fdt_get_end(void* dtb_base)
{
    return dtb_base + be2le(((fdt_header*)dtb_base)->totalsize);
}

void* fdt_traverse(
    void* dtb_base, char* node_name, char* prop_name, int* prop_len)
{
    fdt_header* header = (fdt_header*)(dtb_base);

    if (be2le(header->magic) != FDT_MAGIC)
        return 0;

    char* structure_head = (char*)header + be2le(header->off_dt_struct);
    char* string_head = (char*)header + be2le(header->off_dt_strings);
    char* current_node = 0;

    while (1) {
        unsigned int token = be2le(*((unsigned int*)structure_head));
        structure_head += sizeof(unsigned int);
        switch (token) {
        case FDT_BEGIN_NODE:
            current_node = structure_head;
            // +1 is for '\0' byte
            structure_head += align4(strlen(structure_head) + 1);
            break;
        case FDT_PROP: {
            fdt_prop* prop = (fdt_prop*)structure_head;
            unsigned int len = be2le(prop->len);
            char* name = string_head + be2le(prop->nameoff);
            structure_head += sizeof(fdt_prop);
            if ((!node_name || strcmp(current_node, node_name) == 0)
                && strcmp(name, prop_name) == 0) {
                *prop_len = len;
                return structure_head;
            }
            structure_head += align4(len);
            break;
        }
        case FDT_END_NODE:
        case FDT_NOP:
            break;
        default:
            return 0;
        }
    }
}

unsigned long long fdt_read_cells(void* address, int cells)
{
    unsigned long long ret = 0;
    for (int i = 0; i < cells; i++)
        ret = (ret << 32) | be2le(*(unsigned int*)(address + i * 4));
    return ret;
}
