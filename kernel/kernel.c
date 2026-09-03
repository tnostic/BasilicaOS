#include "basilica.h"

extern void serial_init(void);

void kmain(struct boot_info* info) {
    serial_init();

    fb_clear();
    fb_puts(" ______                   _   __    _                 \n");
    fb_puts("|_   _ \                 (_) [  |  (_)                \n");
    fb_puts("  | |_) |  ,--.   .--.   __   | |  __   .---.  ,--.   \n");
    fb_puts("  |  __'. `'_\ : ( (`\] [  |  | | [  | / /'`\]`'_\ :  \n");
    fb_puts(" _| |__) |// | |, `'.'.  | |  | |  | | | \__. // | |, \n");
    fb_puts("|_______/ \'-;__/[\__) )[___][___][___]'.___.'\'-;__/ \n");
    fb_puts("======================================================\n");
    idt_init();
    pic_init();
    mem_init(info);
    sti();
    shell_run();
    cli();
    while (1) halt();
}
