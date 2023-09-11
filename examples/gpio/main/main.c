#include "stdio.h"
#include <stdbool.h>
#include <unistd.h>

void app_main(void) 
{
    //sels
    uint8_t pin = 2;

    (*(volatile uint32_t *)(0x60004000 + 0x24)) = ((0x1 << (pin - 32)));
    while(1)
    {
        (*(volatile uint32_t *)(0x60004000 + 0x8)) = ((0x1 << (pin - 32)));
        usleep(1000000);
        (*(volatile uint32_t *)(0x60004000 + 0xC)) = ((0x1 << (pin - 32)));
        usleep(1000000);
    }
}