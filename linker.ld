OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

PHDRS {
    text PT_LOAD FLAGS(4);  /* Read + Execute */
    data PT_LOAD FLAGS(6);  /* Read + Write */
}

SECTIONS {
    . = 1M;  /* Load the kernel at 1MB */

    .multiboot ALIGN(4K) : {
        *(.multiboot)  /* Ensure Multiboot header is at the beginning */
    } :text

    .text ALIGN(4K) : {
        *(.text)       /* Code */
        *(.rodata)     /* Read-only data */
    } :text

    .data ALIGN(4K) : {
        *(.data)       /* Writable data */
        *(.bss)        /* Uninitialized data */
    } :data
}

