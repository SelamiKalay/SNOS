# =============================================================================
#  SNOS Makefile
#  Gereksinimler (MSYS2 üzerinden):
#    pacman -S mingw-w64-i686-cross-i686-elf-gcc nasm grub xorriso qemu
# =============================================================================

TARGET   := snos
ARCH     := i686-elf

CC       := $(ARCH)-g++
AS       := nasm
LD       := $(ARCH)-ld

CFLAGS   := -std=c++17 -ffreestanding -fno-exceptions -fno-rtti \
            -O2 -Wall -Wextra \
            -I./include -I./kernel

ASFLAGS  := -f elf32

LDFLAGS  := -T linker.ld -nostdlib

# ---- Kaynak dosyaları ----
ASM_SRCS := boot/boot.asm \
            kernel/isr.asm

CPP_SRCS := kernel/kernel.cpp \
            kernel/gdt.cpp \
            kernel/idt.cpp \
            kernel/pic.cpp \
            kernel/memory/pmm.cpp \
            kernel/memory/heap.cpp \
            kernel/drivers/vesa.cpp \
            kernel/drivers/keyboard.cpp \
            kernel/drivers/mouse.cpp \
            kernel/drivers/rtc.cpp \
            kernel/gui/font.cpp \
            kernel/gui/framebuffer.cpp \
            kernel/gui/cursor.cpp \
            kernel/gui/logconsole.cpp \
            kernel/gui/desktop.cpp \
            kernel/fs/ramdisk.cpp \
            kernel/apps/terminal.cpp \
            kernel/apps/snos_note.cpp

# ---- Build dizini ----
BUILD_DIR := build
ISO_DIR   := iso_root

# ---- Nesne dosyaları ----
ASM_OBJS := $(patsubst %.asm,$(BUILD_DIR)/%.asm.o,$(ASM_SRCS))
CPP_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CPP_SRCS))
ALL_OBJS := $(ASM_OBJS) $(CPP_OBJS)

.PHONY: all clean run iso dirs

all: dirs $(BUILD_DIR)/$(TARGET).elf

dirs:
	@mkdir -p $(BUILD_DIR)/boot
	@mkdir -p $(BUILD_DIR)/kernel/memory
	@mkdir -p $(BUILD_DIR)/kernel/drivers
	@mkdir -p $(BUILD_DIR)/kernel/gui
	@mkdir -p $(BUILD_DIR)/kernel/fs
	@mkdir -p $(BUILD_DIR)/kernel/apps

# Assembly derleme
$(BUILD_DIR)/%.asm.o: %.asm | dirs
	$(AS) $(ASFLAGS) $< -o $@

# C++ derleme
$(BUILD_DIR)/%.o: %.cpp | dirs
	$(CC) $(CFLAGS) -c $< -o $@

# Linkleme
$(BUILD_DIR)/$(TARGET).elf: $(ALL_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@
	@echo ">>> Kernel hazir: $@"

# ---- Limine ISO oluştur (Sadece Legacy BIOS için) ----
LIMINE_URL := https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary

$(BUILD_DIR)/limine:
	@rm -rf $@
	@mkdir -p $@
	@echo ">>> Limine bootloader indiriliyor..."
	curl -sLo $@/limine-cd.bin $(LIMINE_URL)/limine-cd.bin
	curl -sLo $@/limine.sys $(LIMINE_URL)/limine.sys

iso: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)/limine
	@rm -rf $(ISO_DIR)
	@mkdir -p $(ISO_DIR)
	@mkdir -p $(ISO_DIR)/boot/limine
	
	@cp $(BUILD_DIR)/$(TARGET).elf $(ISO_DIR)/boot/snos.elf
	@cp $(BUILD_DIR)/limine/limine-cd.bin $(ISO_DIR)/boot/limine/
	@cp $(BUILD_DIR)/limine/limine.sys $(ISO_DIR)/boot/limine/
	
	@echo "TIMEOUT=0" > $(ISO_DIR)/limine.cfg
	@echo ":SNOS v0.1 by Selami" >> $(ISO_DIR)/limine.cfg
	@echo "    PROTOCOL=multiboot1" >> $(ISO_DIR)/limine.cfg
	@echo "    KERNEL_PATH=boot:///boot/snos.elf" >> $(ISO_DIR)/limine.cfg

	xorriso -as mkisofs -b boot/limine/limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		$(ISO_DIR) -o $(BUILD_DIR)/snos.iso
	@echo ">>> ISO hazir: $(BUILD_DIR)/snos.iso"

# ---- QEMU çalıştır ----
run: iso
	qemu-system-i386 \
		-cdrom $(BUILD_DIR)/snos.iso \
		-m 128M \
		-vga std \
		-serial stdio \
		-no-reboot

# ---- Temizlik ----
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)
	@echo ">>> Temizlendi."
