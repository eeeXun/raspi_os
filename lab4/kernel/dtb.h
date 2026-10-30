#pragma once

void* fdt_get_end(void* dtb_base);
void* fdt_traverse(
    void* dtb_base, char* node_name, char* prop_name, int* prop_len);
unsigned long long fdt_read_cells(void* address, int cells);
