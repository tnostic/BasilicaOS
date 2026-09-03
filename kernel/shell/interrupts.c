#include "../basilica.h"

#define IDT_ENTRIES 256
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

struct idt_entry {
    uint16_t off_lo; uint16_t sel; uint8_t ist; uint8_t attr;
    uint16_t off_mid; uint32_t off_hi; uint32_t zero;
} __attribute__((packed));

struct idt_ptr { uint16_t limit; uint64_t base; } __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;
extern void* isr_table[];

static void set_gate(uint8_t n, uint64_t base, uint16_t sel, uint8_t attr) {
    idt[n].off_lo = base & 0xFFFF;
    idt[n].off_mid = (base >> 16) & 0xFFFF;
    idt[n].off_hi = (base >> 32) & 0xFFFFFFFF;
    idt[n].sel = sel; idt[n].ist = 0; idt[n].attr = attr; idt[n].zero = 0;
}

void pic_init(void) {
    outb(PIC1_CMD, 0x11); outb(PIC2_CMD, 0x11);
    outb(PIC1_DATA, 0x20); outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04); outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01); outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0xFD);
    outb(PIC2_DATA, 0xFF);
}

void idt_init(void) {
    idtp.limit = sizeof(idt) - 1; idtp.base = (uint64_t)&idt;
    for (int i = 0; i < IDT_ENTRIES; i++) set_gate(i, 0, 0, 0);
    for (int i = 0; i < 48; i++) set_gate(i, (uint64_t)isr_table[i], 0x08, 0x8E);
    __asm__ __volatile__("lidt %0" :: "m"(idtp));
}

void interrupt_handler(uint64_t vec, uint64_t err) {
    if (vec >= 32 && vec <= 47) {
        if (vec == 33) keyboard_handler();

            if (vec >= 40) outb(PIC2_CMD, 0x20);
            outb(PIC1_CMD, 0x20);
        return;
    }

    fb_puts("\n!!! KERNEL PANIC!!!\n");
    cli();
    while (1) halt();
}
