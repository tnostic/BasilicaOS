#include "../basilica.h"

#define KBD_PORT 0x60
#define BUFFER_SIZE 256

static char kbd_buffer[BUFFER_SIZE];
static volatile int buf_head = 0;
static volatile int buf_tail = 0;
static int shift_pressed = 0;

// Query for keyboard
static const char scancode_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

static const char scancode_ascii_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

void keyboard_handler(void) {
    uint8_t scancode = inb(KBD_PORT);

    if (scancode == 0x2A || scancode == 0x36) shift_pressed = 1;
        else if (scancode == 0xAA || scancode == 0xB6) shift_pressed = 0;

            if (scancode < 0x80) {
                char c = shift_pressed ? scancode_ascii_shift[scancode] : scancode_ascii[scancode];
                if (c) {
                    kbd_buffer[buf_head++] = c;
                    buf_head %= BUFFER_SIZE;
                }
            }
}

char getchar(void) {
    while (buf_head == buf_tail) { halt(); }
    char c = kbd_buffer[buf_tail++];
    buf_tail %= BUFFER_SIZE;
    sti();

    return c;
}
