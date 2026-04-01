#include "terminal.h"
#include "snos_note.h"
#include "../gui/framebuffer.h"
#include "../gui/logconsole.h"
#include "../drivers/vesa.h"
#include "../drivers/keyboard.h"
#include "../fs/ramdisk.h"
#include "../kernel/pic.h"
#include <io.h>
#include <string.h>

#define TERM_X       50
#define TERM_Y       60
#define TERM_W       700
#define TERM_H       480
#define TERM_COLS    87          // (700-8)/8
#define TERM_ROWS    27          // (480-16)/16 - 1 (prompt satırı)
#define TERM_BG      0xEE0A0A0Au
#define TERM_FG      0xFFCCFFCC  // yeşil metin
#define TERM_PROMPT  0xFF4FC3F7u // mavi prompt

// Her satır için buffer
static char   lines[TERM_ROWS + 1][TERM_COLS + 1];
static int    num_lines = 0;

static char   input_buf[TERM_COLS + 1];
static int    input_len = 0;

static bool   term_active = false;

// ==============================
//  Çıktı satırı ekle
// ==============================
static void term_puts(const char* msg) {
    logconsole_print(msg);

    // Satır kaydırma
    if (num_lines >= TERM_ROWS) {
        for (int i = 0; i < TERM_ROWS - 1; i++)
            strcpy(lines[i], lines[i + 1]);
        num_lines = TERM_ROWS - 1;
    }
    // Uzun satırı böl
    int len = (int)strlen(msg);
    int i = 0;
    while (i < len) {
        int chunk = len - i;
        if (chunk > TERM_COLS) chunk = TERM_COLS;
        for (int j = 0; j < chunk; j++)
            lines[num_lines][j] = msg[i + j];
        lines[num_lines][chunk] = '\0';
        num_lines++;
        if (num_lines >= TERM_ROWS) {
            for (int k = 0; k < TERM_ROWS - 1; k++)
                strcpy(lines[k], lines[k + 1]);
            num_lines = TERM_ROWS - 1;
        }
        i += chunk;
    }
}

// ==============================
//  Komut işleme
// ==============================
static void process_cmd(const char* cmd) {
    // help
    if (strcmp(cmd, "help") == 0) {
        term_puts("Komutlar: help, version, clear, shutdown,");
        term_puts("          ls, mkdir <ad>, rm <ad>,");
        term_puts("          cat <dosya>, touch <dosya>,");
        term_puts("          note <dosya>, exit");
        return;
    }
    // version
    if (strcmp(cmd, "version") == 0) {
        term_puts("SNOS v0.1 – Gelistirici: Selami");
        term_puts("Mimari: x86-32 | Boot: GRUB Multiboot");
        return;
    }
    // clear
    if (strcmp(cmd, "clear") == 0) {
        num_lines = 0;
        return;
    }
    // shutdown
    if (strcmp(cmd, "shutdown") == 0) {
        term_puts("Sistem kapatiliyor...");
        term_puts("SNOS kapatiliyor. Bilgisayarin gucunu kesebilirsiniz...");
        logconsole_print("ACPI shutdown gonderiliyor.");
        snos_shutdown();
        return;
    }
    // ls
    if (strcmp(cmd, "ls") == 0) {
        const char* names[RD_MAX_FILES];
        int n = rd_list(names, RD_MAX_FILES);
        if (n == 0) { term_puts("(bos)"); return; }
        char buf[RD_MAX_NAME + 8];
        for (int i = 0; i < n; i++) {
            RdFile* f = rd_find(names[i]);
            if (f->is_dir) {
                strcpy(buf, "[DIR] "); strcat(buf, names[i]);
            } else {
                strcpy(buf, "      "); strcat(buf, names[i]);
            }
            term_puts(buf);
        }
        return;
    }
    // mkdir <ad>
    if (strncmp(cmd, "mkdir ", 6) == 0) {
        const char* name = cmd + 6;
        if (rd_create(name, true)) {
            char buf[80]; strcpy(buf, "Klasor olusturuldu: "); strcat(buf, name);
            term_puts(buf);
            logconsole_print(buf);
        } else term_puts("HATA: Disk dolu!");
        return;
    }
    // touch <dosya>
    if (strncmp(cmd, "touch ", 6) == 0) {
        const char* name = cmd + 6;
        if (rd_create(name, false)) {
            char buf[80]; strcpy(buf, "Dosya olusturuldu: "); strcat(buf, name);
            term_puts(buf);
        } else term_puts("HATA: Disk dolu!");
        return;
    }
    // rm <ad>
    if (strncmp(cmd, "rm ", 3) == 0) {
        const char* name = cmd + 3;
        if (rd_delete(name)) {
            char buf[80]; strcpy(buf, "Silindi: "); strcat(buf, name);
            term_puts(buf);
        } else term_puts("HATA: Dosya bulunamadi!");
        return;
    }
    // cat <dosya>
    if (strncmp(cmd, "cat ", 4) == 0) {
        const char* name = cmd + 4;
        RdFile* f = rd_find(name);
        if (!f || f->is_dir) { term_puts("HATA: Dosya yok."); return; }
        // Satır satır yazdır
        const char* p = f->data;
        char line_tmp[TERM_COLS + 1];
        int li = 0;
        while (*p) {
            if (*p == '\n' || li >= TERM_COLS) {
                line_tmp[li] = '\0';
                term_puts(line_tmp);
                li = 0;
            } else {
                line_tmp[li++] = *p;
            }
            p++;
        }
        if (li > 0) { line_tmp[li]='\0'; term_puts(line_tmp); }
        return;
    }
    // note <dosya>
    if (strncmp(cmd, "note ", 5) == 0) {
        const char* name = cmd + 5;
        snos_note_open(name);
        return;
    }
    // exit
    if (strcmp(cmd, "exit") == 0) {
        term_active = false;
        return;
    }

    if (cmd[0] != '\0') {
        char buf[80];
        strcpy(buf, "Bilinmeyen komut: "); strcat(buf, cmd);
        term_puts(buf);
    }
}

// ==============================
//  Render
// ==============================
void terminal_render() {
    if (!term_active) return;

    // Terminal arka planı
    vesa_fill_rect(TERM_X, TERM_Y, TERM_W, TERM_H, TERM_BG);
    vesa_draw_rect(TERM_X, TERM_Y, TERM_W, TERM_H, 0xFF2979FFu);

    // Başlık çubuğu
    vesa_fill_rect(TERM_X + 1, TERM_Y + 1, TERM_W - 2, 20, 0xFF1A1A2Eu);
    fb_draw_string(TERM_X + 8, TERM_Y + 3, "SNOS Shell v0.1", 0xFF90CAF9u, COLOR_TRANSPARENT);
    // Kapat butonu
    vesa_fill_rect(TERM_X + TERM_W - 22, TERM_Y + 4, 14, 14, 0xFF880000u);
    fb_draw_string(TERM_X + TERM_W - 19, TERM_Y + 5, "x", COLOR_WHITE, COLOR_TRANSPARENT);

    // Geçmiş satırları
    for (int i = 0; i < num_lines; i++) {
        fb_draw_string(TERM_X + 6, TERM_Y + 24 + i * 17, lines[i], TERM_FG, COLOR_TRANSPARENT);
    }

    // Prompt + input
    int prompt_y = TERM_Y + TERM_H - 22;
    vesa_fill_rect(TERM_X + 1, prompt_y - 2, TERM_W - 2, 20, 0xFF090909u);
    fb_draw_string(TERM_X + 6, prompt_y, "SNOS:> ", TERM_PROMPT, COLOR_TRANSPARENT);
    fb_draw_string(TERM_X + 6 + 7 * 8, prompt_y, input_buf, TERM_FG, COLOR_TRANSPARENT);
    // İmleç yanıp sönme yoktu, statik çizilir
    int cursor_px = TERM_X + 6 + (7 + input_len) * 8;
    vesa_fill_rect(cursor_px, prompt_y, 2, 14, COLOR_WHITE);
}

// ==============================
//  Ana döngü
// ==============================
void terminal_init() { term_active = true; num_lines = 0; input_len = 0; }

void terminal_run() {
    term_active = true;
    term_puts("SNOS Shell - 'help' yazin.");
    term_puts("Cikis: exit");

    while (term_active) {
        terminal_render();
        vesa_swap();

        char c = keyboard_getchar();

        if (c == '\n') {
            input_buf[input_len] = '\0';
            // echo
            char echo_line[TERM_COLS + 8];
            strcpy(echo_line, "SNOS:> ");
            strcat(echo_line, input_buf);
            term_puts(echo_line);
            process_cmd(input_buf);
            input_len = 0;
            input_buf[0] = '\0';
        } else if (c == '\b') {
            if (input_len > 0) {
                input_len--;
                input_buf[input_len] = '\0';
            }
        } else if (input_len < TERM_COLS) {
            input_buf[input_len++] = c;
            input_buf[input_len]   = '\0';
        }
    }
}
