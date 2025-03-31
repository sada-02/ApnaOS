ENTRY(kernel_main)
SECTIONS {
    . = 0x00100000;  /* load address of the kernel (example: 1MB) */

    /* Place the multiboot header first */
    .multiboot :
    {
        KEEP(*(.multiboot))
    }

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}
