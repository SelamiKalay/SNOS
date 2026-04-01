#include "snos_note.h"
#include "../gui/framebuffer.h"
#include "../gui/logconsole.h"
#include "../drivers/vesa.h"
#include "../drivers/keyboard.h"
#include "../fs/ramdisk.h"
#include <string.h>

#define NOTE_X   80
#define NOTE_Y   50
#define NOTE_W   640
#define NOTE_H   500
#define NOTE_COLS 79
#define NOTE_ROWS 28

static char editor_buf[RD_MAX_DATA];
static int  editor_len = 0;
static int  edit_scroll = 0;   // kaydırma satırı

// Satırlara böl
#define MAX_LINES 128
static char  note_lines[MAX_LINES][NOTE_COLS + 1];
static int   note_lcount = 0;
static int   cursor_line = 0, cursor_col = 0;

static void rebuild_lines() {
    note_lcount = 0;
    int li = 0;
    const char* p = editor_buf;
    while (*p && note_lcount < MAX_LINES) {
        if (*p == '\n' || li >= NOTE_COLS) {
            note_lines[note_lcount][li] = '\0';
            note_lcount++;
            li = 0;
            if (*p == '\n') p++;
        } else {
            note_lines[note_lcount][li++] = *p++;
        }
    }
    if (li > 0 || note_lcount == 0) {
        note_lines[note_lcount][li] = '\0';
        note_lcount++;
    }
}

static void render_note(const char* filename) {
    vesa_fill_rect(NOTE_X, NOTE_Y, NOTE_W, NOTE_H, 0xFF0F0F1Au);
    vesa_draw_rect(NOTE_X, NOTE_Y, NOTE_W, NOTE_H, 0xFF2979FFu);

    // Başlık
    vesa_fill_rect(NOTE_X+1, NOTE_Y+1, NOTE_W-2, 20, 0xFF111130u);
    char title[80];
    strcpy(title, "SNOS-Note: ");
    strcat(title, filename);
    fb_draw_string(NOTE_X + 8, NOTE_Y + 3, title, 0xFF90CAF9u, COLOR_TRANSPARENT);
    fb_draw_string(NOTE_X + NOTE_W - 200, NOTE_Y + 3,
                   "Ctrl+S=Kaydet  Ctrl+Q=Cikis", 0xFF555577u, COLOR_TRANSPARENT);

    // Satırları çiz
    for (int i = 0; i < NOTE_ROWS; i++) {
        int li = edit_scroll + i;
        if (li >= note_lcount) break;
        uint32_t fg = (li == cursor_line) ? COLOR_WHITE : 0xFF99BBFFu;
        fb_draw_string(NOTE_X + 6, NOTE_Y + 24 + i * 17, note_lines[li], fg, COLOR_TRANSPARENT);
        // İmleç
        if (li == cursor_line) {
            int cx = NOTE_X + 6 + cursor_col * 8;
            int cy = NOTE_Y + 24 + i * 17;
            vesa_fill_rect(cx, cy, 2, 14, 0xFF2979FFu);
        }
    }

    // Alt durum çubuğu
    vesa_fill_rect(NOTE_X+1, NOTE_Y+NOTE_H-18, NOTE_W-2, 17, 0xFF050510u);
    char status[80];
    strcpy(status, "Satir: ");
    char tmp[10]; itoa(cursor_line+1, tmp, 10);
    strcat(status, tmp);
    strcat(status, "  Uzunluk: ");
    itoa(editor_len, tmp, 10);
    strcat(status, tmp);
    strcat(status, " bayt");
    fb_draw_string(NOTE_X + 8, NOTE_Y + NOTE_H - 16, status, 0xFF4CAF50u, COLOR_TRANSPARENT);
}

void snos_note_open(const char* filename) {
    RdFile* f = rd_find(filename);
    bool is_new = (f == NULL);
    if (is_new) f = rd_create(filename, false);
    if (!f) return;

    if (!is_new) {
        memcpy(editor_buf, f->data, f->size + 1);
        editor_len = (int)f->size;
    } else {
        editor_buf[0] = '\0';
        editor_len = 0;
    }

    cursor_line = 0; cursor_col = 0; edit_scroll = 0;
    rebuild_lines();

    char msg[80]; strcpy(msg, "SNOS-Note acildi: "); strcat(msg, filename);
    logconsole_print(msg);

    bool running = true;
    while (running) {
        render_note(filename);
        vesa_swap();

        char c = keyboard_getchar();
        bool ctrl = key_ctrl_pressed();

        if (ctrl && c == 's') {
            // Kaydet
            rd_write(f, editor_buf, (uint32_t)editor_len);
            logconsole_print("Dosya kaydedildi.");
            continue;
        }
        if (ctrl && c == 'q') {
            running = false;
            logconsole_print("SNOS-Note kapatildi.");
            continue;
        }
        if (ctrl && c == 'c') {
            // Mevcut satırı kopyala
            int li = cursor_line;
            clipboard_copy(note_lines[li], strlen(note_lines[li]));
            logconsole_print("Satir kopyalandi.");
            continue;
        }
        if (ctrl && c == 'v') {
            // Yapıştır
            char paste[4096]; size_t plen;
            clipboard_paste(paste, &plen);
            if (editor_len + (int)plen < RD_MAX_DATA - 1) {
                // Mevcut konuma ekle
                for (size_t k = 0; k < plen; k++) {
                    editor_buf[editor_len++] = paste[k];
                }
                editor_buf[editor_len] = '\0';
                rebuild_lines();
            }
            continue;
        }

        // Geri tuş
        if (c == '\b') {
            if (editor_len > 0) {
                editor_len--;
                editor_buf[editor_len] = '\0';
                if (cursor_col > 0) cursor_col--;
                rebuild_lines();
            }
            continue;
        }

        // Normal karakter ekle
        if (editor_len < RD_MAX_DATA - 1) {
            editor_buf[editor_len++] = c;
            editor_buf[editor_len]   = '\0';
            if (c == '\n') {
                cursor_line++;
                cursor_col = 0;
                if (cursor_line - edit_scroll >= NOTE_ROWS)
                    edit_scroll++;
            } else {
                cursor_col++;
            }
            rebuild_lines();
        }
    }
}
