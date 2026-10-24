#include "program.h"

#define USER_PROGRAM_BASE 0x200000
#define USER_STACK_TOP 0x300000

void run_user_program(char* data, unsigned int size)
{
    char* entry = (char*)USER_PROGRAM_BASE;
    for (unsigned int i = 0; i < size; i++)
        entry[i] = data[i];
    enter_el0(USER_PROGRAM_BASE, USER_STACK_TOP);
}
