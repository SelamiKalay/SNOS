/* =============================================================================
   SNOS – linker.ld
   Kernel'i fiziksel 1 MB adresine (0x00100000) yükler.
   GRUB Multiboot için .multiboot section en başa yerleştirilir.
   ============================================================================= */

OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    /* Çekirdek 1 MB'tan başlar */
    . = 0x00100000;

    /* Multiboot başlığı en önce */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* Kernel sonunu işaretle – PMM bu adresten itibaren ayırır */
    kernel_end = .;
}
