#include "basilica.h"

struct free_page {
    struct free_page* next;
};

static struct free_page* freelist = NULL;
void* page_alloc(void);
void page_free(void* ptr);

void mem_init(struct boot_info* info) {
    struct e820_entry* ent = (struct e820_entry*)(uintptr_t)info->e820_addr;
    for (uint32_t i = 0; i < info->e820_count; i++) {
        if (ent[i].type != E820_AVAILABLE) continue;
        uint64_t base = ent[i].base;
        uint64_t end  = base + ent[i].length;

        if (end <= 0x200000) continue;
        if (base < 0x200000) base = 0x200000;
        base = (base + PAGE_SIZE - 1) & PAGE_MASK;
        end  = end & PAGE_MASK;

        for (uint64_t p = base; p + PAGE_SIZE <= end; p += PAGE_SIZE)
            page_free((void*)(uintptr_t)p);
    }
}

void* page_alloc(void) {
    if (!freelist) return NULL;
    struct free_page* p = freelist;
    freelist = p->next;
    uint64_t* q = (uint64_t*)p;
    for (int i = 0; i < PAGE_SIZE / 8; i++) q[i] = 0;
    return p;
}

void page_free(void* ptr) {
    if (!ptr) return;
    struct free_page* p = ptr;
    p->next = freelist;
    freelist = p;
}
