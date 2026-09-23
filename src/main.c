#include <stdint.h>

#define UART_FIFO_REG (*(volatile uint32_t *)0x3FF40000)
#define UART_STATUS_REG (*(volatile uint32_t *)0x3FF4001C)

#define TIMG0_WDTWPROTECT_REG (*(volatile uint32_t *)0x3FF5F064)
#define TIMG0_WDTFEED_REG (*(volatile uint32_t *)0x3FF5F060)
#define WDT_WKEY_VALUE 0x50D83AA1

void uart_putc(char c) {
    while (((UART_STATUS_REG >> 16) & 0x3FF) >= 128) { }
    UART_FIFO_REG = c;
}

void feed_watchdog(void) {
    TIMG0_WDTWPROTECT_REG = WDT_WKEY_VALUE;
    TIMG0_WDTFEED_REG = 1;
}

uint32_t read_u32_le(uint8_t *buffer, int i) {
    uint32_t output = 0;

    output |= buffer[i];
    output |= buffer[i+1] << 8;
    output |= buffer[i+2] << 16;
    output |= buffer[i+3] << 24;
    return output;
}

uint16_t read_u16_le(uint8_t *buffer, int i) {
    uint16_t output = 0;

    output |= buffer[i];
    output |= buffer[i+1] << 8;
    return output;
}

typedef struct partition_entry_t {
    uint16_t magic;
    uint8_t  type;
    uint8_t  subtype;
    uint32_t offset;
    uint32_t size;
    char     name[16];
    uint32_t flags;
} partition_entry_t;

uint8_t test_partition_bytes[32] = {
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00,
    0x00, 0x50, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void print_hex32(uint32_t value)
{
    uint32_t mask = 0xF0000000;

    for (int i = 0; i < 8; i++) {
        uint8_t nibble = (value & mask) >> (28 - 4 * i);

        if (nibble <= 9)
            uart_putc('0' + nibble);
        else
            uart_putc('A' + (nibble - 10));

        mask >>= 4;
    }
}
void print_hex16(uint16_t value)
{
    uint16_t mask = 0xF000;

    for (int i = 0; i < 4; i++) {
        uint8_t nibble = (value & mask) >> (12 - 4 * i);

        if (nibble <= 9)
            uart_putc('0' + nibble);
        else
            uart_putc('A' + (nibble - 10));

        mask >>= 4;
    }
}
void print_hex8(uint8_t value)
{
    uint8_t mask = 0xF0;

    for (int i = 0; i < 2; i++) {
        uint8_t nibble = (value & mask) >> (4 - 4 * i);

        if (nibble <= 9)
            uart_putc('0' + nibble);
        else
            uart_putc('A' + (nibble - 10));

        mask >>= 4;
    }
}

void bootloader_main(void) {   
    partition_entry_t partition_entry;
    partition_entry.magic = read_u16_le(test_partition_bytes, 0);
    partition_entry.type = test_partition_bytes[2];
    partition_entry.subtype = test_partition_bytes[3];
    for (int i = 0; i < 16; i++) {
        partition_entry.name[i] = test_partition_bytes[12 + i];
    }    
    partition_entry.offset = read_u32_le(test_partition_bytes, 4);
    partition_entry.size = read_u32_le(test_partition_bytes, 8);
    partition_entry.flags = read_u32_le(test_partition_bytes, 28);

    print_hex16(partition_entry.magic);
    uart_putc(',');
    for (int i=0;i<16;i++) {
        uart_putc(partition_entry.name[i]);
    }
    uart_putc(',');
    print_hex8(partition_entry.subtype);
    uart_putc(',');
    print_hex32(partition_entry.size);
    uart_putc(',');
    print_hex32(partition_entry.offset);
    uart_putc(',');
    print_hex32(partition_entry.flags);

    while (1) {
        feed_watchdog();
    }
}