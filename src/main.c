#include <stdint.h>

#define UART_FIFO_REG (*(volatile uint32_t *)0x3FF40000)
#define UART_STATUS_REG (*(volatile uint32_t *)0x3FF4001C)

#define TIMG0_WDTWPROTECT_REG (*(volatile uint32_t *)0x3FF5F064)
#define TIMG0_WDTFEED_REG     (*(volatile uint32_t *)0x3FF5F060)
#define WDT_WKEY_VALUE         0x50D83AA1

void uart_putc(char c) {
    while (((UART_STATUS_REG >> 16) & 0x3FF) >= 128) { }
    UART_FIFO_REG = c;
}

void feed_watchdog(void) {
    TIMG0_WDTFEED_REG = WDT_WKEY_VALUE;
    TIMG0_WDTFEED_REG = 1;
}

void bootloader_main(void) {
    uart_putc(100);
    while (1) {
        feed_watchdog();
    }
}