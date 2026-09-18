# SNOS

32-bit x86 mimarisi için sıfırdan yazılmış, bare-metal bir hobi işletim sistemi.
Herhangi bir hazır çekirdek veya standart kütüphane kullanılmadan, freestanding
C++ ve Assembly ile geliştirilmektedir.

> Durum: **v0.1** — erken geliştirme aşamasında, eğitim amaçlı bir proje.

## Özellikler

- **Multiboot uyumlu önyükleme** — Limine (ISO) veya GRUB ile yüklenir,
  800×600×32 grafik modu ister
- **CPU altyapısı** — GDT, IDT, kesme servis rutinleri (ISR) ve PIC yapılandırması
- **Bellek yönetimi**
  - Bitmap tabanlı fiziksel bellek yöneticisi (4 KB çerçeveler, Multiboot bellek haritasından)
  - First-fit kernel heap (32 MB)
- **Sürücüler** — VESA framebuffer, PS/2 klavye, PS/2 fare, RTC (gerçek zamanlı saat)
- **Grafik arayüz** — framebuffer çizim katmanı, bitmap font, fare imleci,
  görev çubuğu ve saat içeren bir masaüstü, log konsolu
- **RAM disk dosya sistemi** — bellekte dosya ve klasör oluşturma/silme
- **Uygulamalar**
  - **Terminal** — `help`, `version`, `clear`, `ls`, `mkdir`, `touch`, `rm`,
    `cat`, `note`, `shutdown`, `exit` komutları
  - **SNOS Note** — RAM disk üzerindeki dosyalar için basit metin düzenleyici

## Kullanılan Teknolojiler

- **C++17** (freestanding, `-fno-exceptions -fno-rtti`) — çekirdeğin tamamı
- **NASM Assembly** — Multiboot giriş noktası ve kesme stub'ları
- **i686-elf çapraz derleyici** (GCC/LD) ve özel linker script
- **Makefile** — derleme, ISO oluşturma ve çalıştırma
- **Limine / GRUB** — önyükleyici
- **QEMU** — test ortamı

## Klasör Yapısı

```
boot/          Multiboot başlığı ve giriş noktası (boot.asm)
kernel/        Çekirdek kaynak kodu
  memory/      Fiziksel bellek yöneticisi ve heap
  drivers/     VESA, klavye, fare, RTC sürücüleri
  gui/         Framebuffer, font, imleç, masaüstü, log konsolu
  fs/          RAM disk dosya sistemi
  apps/        Terminal ve SNOS Note
include/       Freestanding header'lar (stdint, string, io, multiboot)
grub/          GRUB yapılandırması
iso_root/      ISO imajı için geçici dizin (make iso ile oluşturulur)
linker.ld      Linker script
```

## Derleme ve Çalıştırma

Gereksinimler (Windows'ta MSYS2 ile):

```bash
pacman -S mingw-w64-i686-cross-i686-elf-gcc nasm xorriso qemu
```

```bash
make          # çekirdeği derler (build/snos.elf)
make iso      # Limine ile önyüklenebilir ISO oluşturur (build/snos.iso)
make run      # ISO'yu QEMU'da çalıştırır
make clean    # derleme çıktılarını temizler
```

Oluşan `build/snos.iso` imajı QEMU, VirtualBox gibi sanal makinelerde
(Legacy BIOS modu) çalıştırılabilir.

## Motivasyon

Bu proje, bir işletim sisteminin en temel seviyede — bilgisayar açıldığı andan
itibaren — nasıl çalıştığını anlamak ve düşük seviye sistem programlama
becerilerini geliştirmek amacıyla sürdürülen kişisel bir çalışmadır.
