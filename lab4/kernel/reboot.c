#include "gpio.h"
#include "mmio.h"

#define PM_PASSWORD 0x5A000000
#define PM_RSTC (PERIPHERAL_BASE + 0x0010001C)
#define PM_WDOG (PERIPHERAL_BASE + 0x00100024)

void reset(int tick)
{
    mmio_write(PM_RSTC, PM_PASSWORD | 0x20);   // full reset
    mmio_write(PM_WDOG, PM_PASSWORD | tick);   // watchdog ticks
}
