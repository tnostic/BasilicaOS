#include "../basilica.h"

static int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && *s1 == *s2) { s1++; s2++; n--; }
    return n == 0 ? 0 : *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void gets(char* buf, int max) {
    int i = 0;
    while (1) {
        char c = getchar();

        if (c == '\n') {
            fb_putc('\n');
            buf[i] = '\0';
            return;
        } else if (c == '\b' && i > 0) {
            i--;
            fb_putc('\b');
        } else if (c >= ' ' && c <= '~' && i < max - 1) {
            buf[i++] = c;
            fb_putc(c);
        }
    }
}

void shell_run(void) {
    char buf[128];

    fb_puts("Type 'help' for a list of commands.\n\n");

    while (1) {
        fb_puts("basilica> ");
        gets(buf, 128);

        if (buf[0] == '\0') {
            continue;
        } else if (strncmp(buf, "echo ", 5) == 0) {
            fb_puts(buf + 5);
            fb_puts("\n");
        } else if (strcmp(buf, "clear") == 0) {
            fb_clear();
        } else if (strcmp(buf, "info") == 0) {
            fb_puts("BasilicaOS x86_64\n");
            fb_puts("Status: Memory initialized, Interrupts Active.\n");
        } else if (strcmp(buf, "matrix") == 0) {
            fb_set_color(2, 0);
            fb_clear();
            fb_puts("Hacker stuff.....\n");
        } else if (strcmp(buf, "normal") == 0) {
            fb_set_color(15, 0);
            fb_clear();
        } else if (strcmp(buf, "help") == 0) {
            fb_puts("Available commands:\n");
            fb_puts("  echo <text>  - Prints the text back to you\n");
            fb_puts("  clear        - Clears the screen\n");
            fb_puts("  matrix       - Im greening\n");
            fb_puts("  normal       - Restore normal colors\n");
            fb_puts("  info         - Shows system info\n");
            fb_puts("  help         - Shows this menu\n");
        } else {
            fb_puts("Unknown command: ");
            fb_puts(buf);
            fb_puts("\n");
        }
    }
}
