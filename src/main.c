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

uint8_t partition[3][32] = {
    {0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00,
    0x00, 0x50, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xAA, 0x50, 0x01, 0x00, 0x00, 0xE0, 0x00, 0x00,
    0x00, 0x20, 0x00, 0x00, 0x6F, 0x74, 0x61, 0x64,
    0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x14, 0x00, 0x61, 0x70, 0x70, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
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
    for (int i=0;i<3;i++) {
        partition_entry.magic = read_u16_le(partition[i], 0);
        partition_entry.type = partition[i][2];

        if (partition_entry.magic == 0x50AA && partition_entry.type == 0x00) {
            for (int j = 0; j < 16; j++) {
                partition_entry.name[j] = partition[i][12 + j];
            }
            partition_entry.subtype = partition[i][3];
            partition_entry.offset = read_u32_le(partition[i], 4);
            partition_entry.size = read_u32_le(partition[i], 8);
            partition_entry.flags = read_u32_le(partition[i], 28);       
            break; 
        }
        
    }


    print_hex16(partition_entry.magic);
    uart_putc(',');
    for (int j = 0; j < 16; j++) {
        if (partition_entry.name[j] == '\0')
            break;

        uart_putc(partition_entry.name[j]);
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