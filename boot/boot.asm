; =============================================================================
;  SNOS - boot.asm
;  Multiboot v1 uyumlu giriş noktası.
;  GRUB bu dosyayı yükler, 16 KB stack kurar, kernel_main'e atlar.
; =============================================================================

MBALIGN  equ 1 << 0          ; modülleri sayfa sınırına hizala
MEMINFO  equ 1 << 1          ; bellek haritasını ver
VIDMODE  equ 1 << 2          ; video modunu yapılandır
FLAGS    equ MBALIGN | MEMINFO | VIDMODE
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

; ---------------------------------------------------------------------------
;  Multiboot başlığı – ilk 8384 bayt içinde, 4-bayt hizalı olmalı
; ---------------------------------------------------------------------------
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    ; ELF ile yüklendiği için adres alanları GRUB tarafından doldurulur
    dd 0, 0, 0, 0, 0
    ; Video isteği (FLAGS bit 2 set)
    dd 0          ; mode_type  : 0 = grafik, 1 = metin
    dd 800        ; width
    dd 600        ; height
    dd 32         ; depth (bpp)

; ---------------------------------------------------------------------------
;  BSS – stack alanı (16 KB)
; ---------------------------------------------------------------------------
section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

; ---------------------------------------------------------------------------
;  Text – gerçek giriş noktası
; ---------------------------------------------------------------------------
section .text
global _start:function (_start.end - _start)
extern kernel_main

_start:
    ; Stack kur
    mov  esp, stack_top

    ; Multiboot argümanlarını sıraya koy (kernel_main(magic, info))
    push ebx            ; multiboot_info_t* (struct adresi)
    push eax            ; sihirli sayı (0x2BADB002 olmalı)

    ; C++ çekirdeğini çağır
    call kernel_main

    ; kernel_main asla dönemez, ama dönerse CPU'yu durdur
    cli
.hang:
    hlt
    jmp .hang
.end:
