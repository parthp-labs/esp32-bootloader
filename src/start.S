.section .text
.global _start

.literal stack_top_ptr, _stack_top
.literal bss_start_ptr, _bss_start
.literal bss_end_ptr, _bss_end

.literal wdtwprotect_addr, 0x3FF480A4
.literal wdtwprotect_unlock, 0x50D83AA1
.literal wdtconfig0_addr, 0x3FF4808C
.literal wdt_disable_val, 0x00000000

_start:
    l32r a1, stack_top_ptr      # a1 = stack pointer
    l32r a2, bss_start_ptr      # a2 = current position
    l32r a3, bss_end_ptr        # a3 = end boundary
    movi a4, 0                  # a4 = 0 (value to store)

bss_zero_loop:
    bgeu  a2, a3, bss_zero_done  # if current >= end, exit loop
    s32i  a4, a2, 0              # store 0 at [current]
    addi  a2, a2, 4              # current += 4
    j     bss_zero_loop

bss_zero_done:
    l32r a2, wdtwprotect_addr     # a2 = &WDTWPROTECT
    l32r a3, wdtwprotect_unlock   # a3 = unlock value
    s32i a3, a2, 0                # WDTWPROTECT = unlock value

    l32r a2, wdtconfig0_addr      # a2 = &WDTCONFIG0
    l32r a3, wdt_disable_val      # a3 = 0
    s32i a3, a2, 0                # WDTCONFIG0 = 0 (disable watchdog)

call0 bootloader_main