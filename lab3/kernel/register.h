#define read_reg(reg)                                                          \
    ({                                                                         \
        unsigned long long __val;                                              \
        asm volatile("mrs %0, " #reg : "=r"(__val));                           \
        __val;                                                                 \
    })

#define write_reg(reg, val)                                                    \
    ({                                                                         \
        unsigned long long __val = (unsigned long long)(val);                  \
        asm volatile("msr " #reg ", %0" : : "r"(__val));                       \
    })
