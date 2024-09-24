ENTRY(Reset_Handler_c0)

MEMORY {
    ROM (rx)	: ORIGIN = 0x10000000, LENGTH = 2048K
    RAM (xrw)	: ORIGIN = 0x20000000, LENGTH = 256K
    RAM4 (rw)   : ORIGIN = 0x2003E000, LENGTH = 4K
    RAM5 (rw)   : ORIGIN = 0x2003F000, LENGTH = 4K
}

SECTIONS {
    boot2 : {
        __boot2_start__ = .;
        KEEP (*(.boot2))
        __boot2_end__ = .;
    } > ROM
    ASSERT(__boot2_end__ - __boot2_start__ == 256,
        "ERROR: Pico second stage bootloader must be 256 bytes in size")

    .text : {
        __vector_org = .;
        KEEP (*(.vector))

        *(EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:) .text*)

        __start = .;
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
    } >ROM

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > ROM

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > ROM
    __exidx_end = .;

    __data_org = .;
    .data : AT(__data_org) {
        __data_start = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        __data_end = .;
    } >RAM
    .bss ALIGN(4) (NOLOAD) : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
        __end = .;
        end = .;
    } >RAM
    
    .vec0 (NOLOAD) : {	/* vec_tbl_c0 をSRAM Bank 4に配置 */
            *(.vector_c0)
    } >RAM4

    .vec1 (NOLOAD) : {	/* vec_tbl_c1 をSRAM Bank 5に配置 */
            *(.vector_c1)
    } >RAM5
}
