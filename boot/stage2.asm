[BITS 16]
[ORG 0x7E00]

%define PML4_ADDR      0x9000
%define PDPT_ADDR      0xA000
%define PDT_ADDR       0xB000
%define GDT64_ADDR     0xC000
%define BOOTINFO_ADDR  0x5000
%define KERNEL_PHYS    0x10000

%define BOOT_DRIVE_ADDR   0x0500
%define E820_COUNT_ADDR   0x0502

stage2_start:
    mov [BOOT_DRIVE_ADDR], dl
    mov si, msg_stage2
    call print_string

    mov si, msg_load
    call print_string
    call load_kernel

    jnc .no_err
    jmp disk_err
.no_err:

    call setup_bootinfo
    call setup_pagetables

    cli
    lgdt [gdt32_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp dword 0x08:protected_mode_32

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

load_kernel:
    mov ah, 0x42
    mov dl, [BOOT_DRIVE_ADDR]
    mov si, dap_kernel
    int 0x13
    ret

setup_bootinfo:
    mov di, BOOTINFO_ADDR
    xor ax, ax
    mov cx, 32
    rep stosw
    mov di, BOOTINFO_ADDR
    mov dword [di + 0], 0x6000
    mov ax, [E820_COUNT_ADDR]
    mov word [di + 8], ax
    ret

setup_pagetables:
    mov di, PML4_ADDR
    xor eax, eax
    mov cx, 12288 / 4
    rep stosd

    mov di, PML4_ADDR
    mov eax, PDPT_ADDR | 3
    mov [di], eax

    mov di, PDPT_ADDR
    mov eax, PDT_ADDR | 3
    mov [di], eax

    mov di, PDT_ADDR
    mov eax, 0x00000083
    mov cx, 512
.loop:
    mov [di], eax
    add di, 8
    add eax, 0x200000
    loop .loop
    ret

disk_err:
    mov si, msg_derr
    call print_string
    jmp $

align 8
gdt32:
    dq 0
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt32_end:
gdt32_desc:
    dw gdt32_end - gdt32 - 1
    dd gdt32

align 8
gdt64:
    dq 0
    dq 0x00AF9A000000FFFF
    dq 0x00AF92000000FFFF
gdt64_end:
gdt64_desc:
    dw gdt64_end - gdt64 - 1
    dd gdt64

msg_stage2:  db 'Stage2 ', 0
msg_load:    db 'Kernel ', 0
msg_done:    db '64bit ', 0
msg_derr:    db 'DiskErr!', 0

align 4
dap_kernel:
    db 0x10, 0x00
    dw 128
    dw 0x0000
    dw 0x1000
    dq 65

[BITS 32]

protected_mode_32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x00090000

    mov word [0xB8000], 0x0F31

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov eax, PML4_ADDR
    mov cr3, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lgdt [gdt64_desc]
    jmp dword 0x08:long_mode_64

[BITS 64]

long_mode_64:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x00080000

    mov word [0xB8002], 0x0F32

    mov rsi, msg_done
    call print_string_64

    mov word [0xB8004], 0x0F33
    mov edi, BOOTINFO_ADDR
    mov rax, KERNEL_PHYS
    jmp rax

print_string_64:
    push rsi
    push rdi
    mov rdi, 0xB8000
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0A
    stosw
    jmp .loop
.done:
    pop rdi
    pop rsi
    ret

times 32768-($-$$) db 0
