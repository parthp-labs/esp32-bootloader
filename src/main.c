#include <stdint.h>

#define BOOT_IRAM_START  0x4009C000
#define BOOT_IRAM_END    0x400A0000

#define UART_FIFO_REG (*(volatile uint32_t *)0x3FF40000)
#define UART_STATUS_REG (*(volatile uint32_t *)0x3FF4001C)

#define TIMG0_WDTWPROTECT_REG (*(volatile uint32_t *)0x3FF5F064)
#define TIMG0_WDTFEED_REG (*(volatile uint32_t *)0x3FF5F060)
#define WDT_WKEY_VALUE 0x50D83AA1

#define PRO_MMU_TABLE ((volatile uint32_t *)0x3FF10000)
#define DROM_VADDR_BASE  0x3F400000
#define IROM_VADDR_BASE  0x400D0000
#define PAGE_SIZE        0x10000
#define MMU_PAGE_SIZE    0x10000
#define PRO_MMU_TABLE    ((volatile uint32_t *)0x3FF10000)
#define DROM_BASE        0x3F400000
#define IROM_BASE        0x40000000

#define IROM_MMU_START   64

#define CMD (volatile uint32_t*) 0x3FF42000
#define ADDR (volatile uint32_t*) 0x3FF42004
#define CTRL (volatile uint32_t*) 0x3FF42008
#define CLOCK (volatile uint32_t*) 0x3FF42018
#define USER (volatile uint32_t*) 0x3FF4201C
#define USER1 (volatile uint32_t*) 0x3FF42020
#define USER2 (volatile uint32_t*) 0x3FF42024
#define MISO_DLEN (volatile uint32_t*) 0x3FF4202C
#define W0 (volatile uint32_t*) 0x3FF42080
#define W1 (volatile uint32_t*) 0x3FF42084
#define PIN (volatile uint32_t *) 0x3FF42034
#define DPORT_PRO_CACHE_CTRL_REG (*(volatile uint32_t *) 0x3FF00040)
#define PRO_CACHE_CTRL (*(volatile uint32_t *)0x3FF00040)
#define CACHE_ENABLE_BIT (1u << 3)

#define SPI_USR          (1u << 18)
#define CTRL_MODE_MASK   ((1u << 26) | (1u << 25) |                 /* bit order        */ \
                          (1u << 24) | (1u << 23) | (1u << 20) |    /* QIO, DIO, QUAD   */ \
                          (1u << 14) | (1u << 13))                  /* DUAL, FASTRD     */

static void spi1_read_flash8(uint32_t flash_addr, uint32_t *w0, uint32_t *w1)
{
    PRO_CACHE_CTRL &= ~(1u << 3);       

    // Wait for any earlier SPI1 transaction 
    while (*CMD & SPI_USR) { }

    *USER = (1u << 31) | (1u << 30) | (1u << 28);  /* cmd + addr + MISO */
    *USER1 = (23u << 26);   /* 24-bit addr, no dummy */
    *USER2 = (7u << 28) | 0x03; /* 8-bit cmd 0x03 (READ) */
    *ADDR = flash_addr << 8;  /* left-aligned 24-bit addr */
    *MISO_DLEN = 63;  // 64 bits
    *PIN = (1u << 1) | (1u << 2);      
    *CTRL &= ~CTRL_MODE_MASK;            
    *CLOCK = (7u << 12) | (3u << 6) | 7u;   //80 MHz / 8 = 10 MHz 

    *W0 = 0;
    *W1 = 0;

    *CMD = SPI_USR;
    while (*CMD & SPI_USR) { }

    *w0 = *W0;
    *w1 = *W1;

    PRO_CACHE_CTRL |= CACHE_ENABLE_BIT;  
}

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

typedef struct {
    uint8_t magic;
    uint8_t segment_count;
    uint8_t spi_mode;
    uint8_t spi_speed;
    uint32_t entry_point;
} image_header_t;

uint8_t image_header_raw[32] = {
    0xE9, 0x05, 0x02, 0x20, 0xAC, 0x29, 0x08, 0x40, 
    0xEE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01,
};

uint8_t segment0[8] = {0x20, 0x00, 0x40, 0x3F, 0x38, 0xD2, 0x00, 0x00};


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

void read_segment(uint8_t segment[8], uint32_t* load_addr, uint32_t* length) {
    *load_addr = 0;
    *length = 0;
    for (int i=0;i<4;i++) {
        *load_addr |= ((uint32_t)segment[i]) << (8*i);
    }
    for (int i=4;i<8;i++) {
        *length |= ((uint32_t)segment[i]) << (8*(i-4));
    }
}

int overlaps_bootloader(uint32_t dest, uint32_t len)
{
    uint32_t end = dest + len;

    // overflow check
    if (end < dest)
        return 1;

    return (dest < BOOT_IRAM_END && end > BOOT_IRAM_START);
}

void bootloader_main(void) {
    uart_putc('A');  
    uint32_t w0, w1;
    uint32_t image_entry = 0x10000;
    spi1_read_flash8(image_entry, &w0, &w1);

    uint32_t application_entry = w1;

    uint8_t segment_count = (w0 >> 8);

    uart_putc('\n');
    uint32_t segment_entry = 0x10000 + 24;
    uint32_t load_addr, length;

    for (int i=0;i<segment_count;i++) {
        spi1_read_flash8(segment_entry, &w0, &w1);
        load_addr = w0;
        length = w1;
        uint32_t data_flash_addr = segment_entry + 8;

        print_hex32(load_addr);
        uart_putc(',');
        print_hex32(length);
        uart_putc('\n');
        
        if (overlaps_bootloader(load_addr, length)) {
            uart_putc('X');
            return;
        }

        if (load_addr >= 0x3F400000 && load_addr <  0x3F800000)
        {
            uint32_t flash_page = data_flash_addr & 0xFFFF0000;

            uint32_t virtual_page = load_addr & 0xFFFF0000;

            uint32_t flash_page_num =flash_page >> 16;

            uint32_t mmu_index = (virtual_page - DROM_BASE) >> 16;

            uint32_t pages = ((load_addr & 0xFFFF) + length + MMU_PAGE_SIZE - 1) / MMU_PAGE_SIZE;

            for (uint32_t p = 0; p < pages; p++) {
                PRO_MMU_TABLE[mmu_index + p] = flash_page_num + p;
            }
        }
        else if (load_addr >= 0x400D0000 && load_addr <  0x40400000)
        {
            int32_t flash_page =data_flash_addr & 0xFFFF0000;

            uint32_t virtual_page = load_addr & 0xFFFF0000;

            uint32_t flash_page_num =
                flash_page >> 16;

            uint32_t mmu_index = IROM_MMU_START + ((virtual_page - IROM_BASE) >> 16);

            uint32_t pages = ((load_addr & 0xFFFF) + length + MMU_PAGE_SIZE - 1) / MMU_PAGE_SIZE;

            for (uint32_t p = 0; p < pages; p++) {
                PRO_MMU_TABLE[mmu_index + p] = flash_page_num + p;
            }
        }
        else
        {
            uint32_t flash_addr = segment_entry + 8;
            uint32_t dest = load_addr;
            uint32_t remaining = length;

            while (remaining >= 8) {
                spi1_read_flash8(flash_addr, &w0, &w1);
                
                *(volatile uint32_t *)(dest + 0) = w0;
                *(volatile uint32_t *)(dest + 4) = w1;

                flash_addr += 8;
                dest += 8;
                remaining -= 8;
            }
        }
        uart_putc('B');
        segment_entry += length + 8;
    }
    uart_putc('C');

    while (1) {
        feed_watchdog();
    }
    
}