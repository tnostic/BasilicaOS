#include "../basilica.h"

#define VGA_BUF     ((volatile uint16_t*)0xB8000)
#define VGA_W       80
#define VGA_H       25
#define SERIAL_PORT 0x3F8

static int row = 0, col = 0;
static uint8_t color = 0x0F;

void fb_set_color(uint8_t fg, uint8_t bg) {
    color = (bg << 4) | (fg & 0x0F);
}

static void update_cursor(void) {
    uint16_t pos = row * VGA_W + col;
    outb(0x3D4, 14); outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15); outb(0x3D5, (uint8_t)(pos & 0xFF));
}

static void scroll(void) {
    if (row >= VGA_H) {

        for (int y = 1; y < VGA_H; y++) {
            for (int x = 0; x < VGA_W; x++) {
                VGA_BUF[(y - 1) * VGA_W + x] = VGA_BUF[y * VGA_W + x];
            }
        }

        for (int x = 0; x < VGA_W; x++) {
            VGA_BUF[(VGA_H - 1) * VGA_W + x] = (uint16_t)' ' | ((uint16_t)color << 8);
        }
        row = VGA_H - 1;
    }
}

void serial_init(void) {
    outb(SERIAL_PORT + 1, 0x00); outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x03); outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03); outb(SERIAL_PORT + 2, 0xC7);
    outb(SERIAL_PORT + 4, 0x0B);
}

static int serial_ready(void) { return inb(SERIAL_PORT + 5) & 0x20; }
void serial_putc(char c) { while (!serial_ready()); outb(SERIAL_PORT, c); }
void serial_puts(const char* s) { while (*s) serial_putc(*s++); }

void fb_clear(void) {
    for (int y = 0; y < VGA_H; y++)
        for (int x = 0; x < VGA_W; x++)
            VGA_BUF[y * VGA_W + x] = (uint16_t)' ' | ((uint16_t)color << 8);
    row = 0; col = 0;
    update_cursor();
}

void fb_putc(char c) {
    if (c == '\n') { col = 0; row++; }
    else if (c == '\b') {
        if (col > 0) col--; else if (row > 0) { row--; col = VGA_W - 1; }
        VGA_BUF[row * VGA_W + col] = (uint16_t)' ' | ((uint16_t)color << 8);
    }
    else {
        VGA_BUF[row * VGA_W + col] = (uint16_t)c | ((uint16_t)color << 8);
        col++;
    }

    if (col >= VGA_W) { col = 0; row++; }

    scroll();
    update_cursor();
}

void fb_puts(const char* s) { while (*s) fb_putc(*s++); }

void print_hex(uint64_t v) {
    serial_puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        int d = (v >> i) & 0xF;
        serial_putc(d < 10 ? '0' + d : 'A' + d - 10);
    }
}
void print_dec(uint64_t v) {
    char b[32]; int i = 0;
    do { b[i++] = '0' + (v % 10); v /= 10; } while (v);
    if (i == 0) serial_putc('0');
    while (i--) serial_putc(b[i]);
}

void printk(const char* fmt, ...) {
    va_list a; va_start(a, fmt);
    while (*fmt) {
        if (*fmt == '%' && *(fmt+1)) {
            fmt++;
            switch (*fmt) {
                case 's': serial_puts(va_arg(a, const char*)); break;
                case 'x': print_hex(va_arg(a, uint64_t)); break;
                case 'd': print_dec(va_arg(a, uint64_t)); break;
                case 'c': serial_putc(va_arg(a, int)); break;
                default:  serial_putc('%'); serial_putc(*fmt);
            }
        } else { serial_putc(*fmt); }
        fmt++;
    }
    va_end(a);
}
