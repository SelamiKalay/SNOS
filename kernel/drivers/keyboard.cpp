#include "keyboard.h"
#include "../kernel/idt.h"
#include "../kernel/pic.h"
#include <string.h>

// PS/2 Set 1 – Türkçe'ye yakın basit US-QWERTY haritası
static const char scancode_map[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t','q',  'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0,   ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};
static const char scancode_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t','Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A',  'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
    0,   ' ',  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#define KEY_BUF_SIZE 64
static char key_buf[KEY_BUF_SIZE];
static int  key_buf_head = 0;
static int  key_buf_tail = 0;

static bool mod_shift = false;
static bool mod_ctrl  = false;
static bool mod_alt   = false;

// Pano tamponu
#define CLIP_SIZE 4096
static char  clip_buf[CLIP_SIZE];
static size_t clip_len = 0;

static void kb_push(char c) {
    int next = (key_buf_tail + 1) % KEY_BUF_SIZE;
    if (next != key_buf_head) {
        key_buf[key_buf_tail] = c;
        key_buf_tail = next;
    }
}

static void keyboard_handler(Registers* r) {
    (void)r;
    uint8_t sc = inb(0x60);

    // Basılı / bırakıldı ayrımı (bit 7)
    bool released = (sc & 0x80) != 0;
    sc &= 0x7F;

    // Modifier tuşları
    if (sc == 0x2A || sc == 0x36) { mod_shift = !released; return; }
    if (sc == 0x1D)                { mod_ctrl  = !released; return; }
    if (sc == 0x38)                { mod_alt   = !released; return; }

    if (released) return;   // bırakma olaylarını yoksay

    // Ctrl kısayolları
    if (mod_ctrl) {
        switch (scancode_map[sc]) {
            case 'c': clipboard_copy(clip_buf, 0); break; // terminal kopyalar
            case 'v': /* paste olayı terminal'de işlenir */ break;
        }
        return;
    }

    char c = mod_shift ? scancode_shift[sc] : scancode_map[sc];
    if (c) kb_push(c);
}

void keyboard_init() {
    irq_register_handler(1, keyboard_handler);
}

char keyboard_getchar() {
    // Tampon dolana kadar bekle (polling – kesme gelince dolar)
    while (key_buf_head == key_buf_tail)
        __asm__ volatile("hlt");
    char c = key_buf[key_buf_head];
    key_buf_head = (key_buf_head + 1) % KEY_BUF_SIZE;
    return c;
}

void clipboard_copy(const char* text, size_t len) {
    if (len == 0) len = strlen(text);
    if (len > CLIP_SIZE - 1) len = CLIP_SIZE - 1;
    memcpy(clip_buf, text, len);
    clip_buf[len] = '\0';
    clip_len = len;
}

void clipboard_paste(char* out_buf, size_t* out_len) {
    memcpy(out_buf, clip_buf, clip_len + 1);
    *out_len = clip_len;
}

bool key_ctrl_pressed()  { return mod_ctrl;  }
bool key_shift_pressed() { return mod_shift; }
bool key_alt_pressed()   { return mod_alt;   }
