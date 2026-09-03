#ifndef BASILICA_H
#define BASILICA_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

struct boot_info {
    uint64_t e820_addr;
    uint32_t e820_count;
    uint32_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint8_t  fb_bpp;
    uint8_t  _pad[3];
};

struct e820_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
    uint64_t _pad;
};

#define E820_AVAILABLE  1
#define PAGE_SIZE       4096
#define PAGE_MASK       (~(PAGE_SIZE - 1))

static inline void outb(uint16_t p, uint8_t v) { __asm__ __volatile__("outb %0, %1" :: "a"(v), "Nd"(p)); }
static inline uint8_t inb(uint16_t p) { uint8_t v; __asm__ __volatile__("inb %1, %0" : "=a"(v) : "Nd"(p)); return v; }
static inline void cli(void) { __asm__ __volatile__("cli"); }
static inline void sti(void) { __asm__ __volatile__("sti"); }
static inline void halt(void) { __asm__ __volatile__("hlt"); }

/* Console */
void serial_puts(const char* str);
void print_dec(uint64_t n);
void print_hex(uint64_t n);
void printk(const char* fmt, ...);
void fb_clear(void);
void fb_putc(char c);
void fb_puts(const char* s);
void fb_set_color(uint8_t fg, uint8_t bg);

/* System */
void idt_init(void);
void pic_init(void);

/* Memory */
void mem_init(struct boot_info* info);
void* page_alloc(void);
void page_free(void* ptr);

/* Keyboard & Shell */
void keyboard_handler(void);
char getchar(void);
void shell_run(void);

#endif
