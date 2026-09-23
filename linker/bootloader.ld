ENTRY(_start)

MEMORY
{
    IRAM (rx) : ORIGIN = 0x40080000, LENGTH = 128K
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
    } > IRAM

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