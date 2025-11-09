/*
 * Linker script for STM32L072CBT6
 * Memory layout:
 *   Flash: 192KB (0x08000000 - 0x0802FFFF)
 *   RAM:   20KB  (0x20000000 - 0x20004FFF)
 */

/* Memory definition */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 192K
    RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Stack size */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Minimum heap size */
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;

/* Section definitions */
SECTIONS
{
    /* Vector table and program code in FLASH */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    /* Program code */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* Initialized data */
    _sidata = LOADADDR(.data);
    
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >RAM AT> FLASH

    /* Uninitialized data (BSS) */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM

    /* User heap section */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >RAM
}

