unsigned int align4(unsigned int num) { return (num + 0b11) & ~(0b11); }

void delay(unsigned long long cycles)
{
    while (cycles--)
        asm volatile("nop");
}
