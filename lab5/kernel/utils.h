#pragma once

#define LEN(arr) (sizeof(arr) / sizeof(arr)[0])
#define INT_MAX 0x7FFFFFFF

static inline unsigned int align4(unsigned int num)
{
    return (num + 0b11) & ~(0b11);
}

static inline void delay(unsigned long long cycles)
{
    while (cycles--)
        asm volatile("nop");
}
