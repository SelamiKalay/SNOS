#!/usr/bin/env python3
"""SNOS QEMU acilis duman testi.

ISO'yu ekransiz QEMU'da acar, cekirdegin masaustunu cizmesini bekler, QEMU
monitoru ile ekran goruntusu alir ve sunlari dogrular:
  * QEMU erken kapanmadi (-no-reboot: triple fault olursa QEMU cikar)
  * cozunurluk, Multiboot basliginda istenen 800x600
  * arka plan kirmizisi (desktop.cpp: 0xE30A17) ekranin cogunu kapliyor
  * alt kenarda gorev cubugu rengi (0x181818) var

Kullanim: python3 tests/qemu_boot_test.py build/snos.iso [cikti.ppm]
(cikti.ppm verilirse yanina cikti.png de yazilir)
"""

import os
import socket
import subprocess
import sys
import struct
import tempfile
import time
import zlib

BOOT_WAIT_S = int(os.environ.get("SNOS_BOOT_WAIT", "15"))
EXPECTED_SIZE = (800, 600)
BACKGROUND = (0xE3, 0x0A, 0x17)
TASKBAR = (0x18, 0x18, 0x18)


def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    # P6\n<w> <h>\n<max>\n<binary>
    parts = data.split(maxsplit=4)
    if parts[0] != b"P6":
        raise ValueError("PPM (P6) bekleniyordu")
    w, h, maxval = int(parts[1]), int(parts[2]), int(parts[3])
    if maxval != 255:
        raise ValueError(f"beklenmeyen maxval {maxval}")
    pixels = parts[4][: w * h * 3]
    return w, h, pixels


def write_png(path, w, h, pixels):
    """Bagimliliksiz PNG yazici (CI artifact'i icin)."""
    raw = b"".join(b"\x00" + pixels[y * w * 3:(y + 1) * w * 3] for y in range(h))

    def chunk(tag, body):
        crc = zlib.crc32(tag + body) & 0xFFFFFFFF
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", crc)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n"
                + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw))
                + chunk(b"IEND", b""))


def monitor_cmd(sock_path, cmd):
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
        s.settimeout(10)
        s.connect(sock_path)
        time.sleep(0.3)
        s.recv(4096)  # karsilama satiri
        s.sendall(cmd.encode() + b"\n")
        time.sleep(1.0)


def main():
    iso = sys.argv[1]
    tmp = tempfile.mkdtemp(prefix="snos-qemu-")
    sock_path = os.path.join(tmp, "monitor.sock")
    shot = sys.argv[2] if len(sys.argv) > 2 else os.path.join(tmp, "screen.ppm")

    qemu = subprocess.Popen([
        "qemu-system-i386",
        "-cdrom", iso,
        "-m", "128M",
        "-vga", "std",
        "-display", "none",
        "-no-reboot",
        "-serial", "null",
        "-monitor", f"unix:{sock_path},server,nowait",
    ])
    try:
        deadline = time.time() + BOOT_WAIT_S
        while time.time() < deadline:
            if qemu.poll() is not None:
                print(f"HATA: QEMU {BOOT_WAIT_S} sn dolmadan kapandi "
                      f"(kod {qemu.returncode}); cekirdek coktu mu (triple fault)?")
                return 1
            time.sleep(0.5)

        monitor_cmd(sock_path, f"screendump {shot}")
        for _ in range(20):
            if os.path.exists(shot) and os.path.getsize(shot) > 0:
                break
            time.sleep(0.5)
        else:
            print("HATA: ekran goruntusu alinamadi")
            return 1
        time.sleep(0.5)
    finally:
        try:
            monitor_cmd(sock_path, "quit")
        except OSError:
            pass
        try:
            qemu.wait(timeout=10)
        except subprocess.TimeoutExpired:
            qemu.kill()

    w, h, px = read_ppm(shot)
    write_png(os.path.splitext(shot)[0] + ".png", w, h, px)
    total = w * h
    bg = sum(1 for i in range(0, len(px), 3) if tuple(px[i:i + 3]) == BACKGROUND)
    bottom = px[(h - 5) * w * 3:(h - 4) * w * 3]  # alttan 5. satir
    taskbar = sum(1 for i in range(0, len(bottom), 3)
                  if tuple(bottom[i:i + 3]) == TASKBAR)

    print(f"Cozunurluk: {w}x{h}")
    print(f"Arka plan kirmizisi: {bg / total:.1%} piksel")
    print(f"Gorev cubugu (alt satir): {taskbar / w:.1%} piksel")

    errors = []
    if (w, h) != EXPECTED_SIZE:
        errors.append(f"cozunurluk {EXPECTED_SIZE} bekleniyordu")
    if bg / total < 0.30:
        errors.append("masaustu arka plani cizilmemis gorunuyor")
    if taskbar / w < 0.50:
        errors.append("gorev cubugu bulunamadi")
    for e in errors:
        print("HATA:", e)
    if not errors:
        print("OK: SNOS acildi ve masaustu cizildi.")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
