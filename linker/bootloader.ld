ENTRY(_start)

MEMORY
{
    APP_IRAM (rx) : ORIGIN = 0x40080000, LENGTH = 112K
    BOOT_IRAM (rx) : ORIGIN = 0x4009C000, LENGTH = 16K

    DRAM (rw) : ORIGIN = 0x3FFE0000, LENGTH = 128K
}

_stack_top = ORIGIN(DRAM) + LENGTH(DRAM);    
SECTIONS {
    .text :
    {
        *(.literal)
        *(.literal.*)
        *(.text)
        *(.text.*)
    } > BOOT_IRAM

    .data : 
    {
        *(.data*)
    } > DRAM

    .bss : 
    {
        _bss_start = .;
        *(.bss*)
        _bss_end = .;
    } > DRAM
}