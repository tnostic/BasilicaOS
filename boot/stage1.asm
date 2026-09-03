[BITS 16]
[ORG 0x7C00]

%define STAGE2_SECTORS    64
%define STAGE2_LBA        1
%define STAGE2_ADDR       0x7E00


%define BOOT_DRIVE_ADDR   0x0500
%define E820_COUNT_ADDR   0x0502

entry:
    jmp bootcode
    nop

OEMName:          db 'BASILICA'
BytesPerSector:   dw 512
SectsPerCluster:  db 1
ReservedSectors:  dw 1
NumFATs:          db 2
RootEntries:      dw 224
TotalSectors16:   dw 2880
Media:            db 0xF0
FATSize16:        dw 9
SectorsPerTrack:  dw 18
NumHeads:         dw 2
HiddenSectors:    dd 0
TotalSectors32:   dd 0
DriveNum:         db 0
Reserved1:        db 0
BootSig:          db 0x29
VolumeID:         dd 0x12345678
VolumeLabel:      db 'BASILICA OS'
FileSystemType:   db 'FAT12   '

bootcode:
    cli
    cld
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00
    mov ds, ax
    mov es, ax
    sti
    mov [BOOT_DRIVE_ADDR], dl

    mov si, msg_boot
    call print_string

    call do_e820
    call enable_a20

    mov si, msg_load
    call print_string
    call load_stage2

    jnc .skip_err
    jmp disk_error
.skip_err:
    jmp 0x0000:STAGE2_ADDR

do_e820:
    mov si, msg_e820
    call print_string
    mov di, 0x6000
    xor ebx, ebx
    xor bp, bp
    mov edx, 0x534D4150
    mov eax, 0xE820
    mov [es:di + 20], dword 1
    mov ecx, 24
    int 0x15
    jc short .done
    cmp eax, edx
    jne short .done
    test ebx, ebx
    je short .done
    jmp short .jmpin
.e820lp:
    mov eax, 0xE820
    mov [es:di + 20], dword 1
    mov ecx, 24
    int 0x15
    jc short .done
    mov edx, 0x534D4150
.jmpin:
    jcxz .skip
    cmp cl, 20
    jbe .notext
    test byte [es:di + 20], 1
    je short .skip
.notext:
    mov ecx, [es:di + 8]
    test ecx, ecx
    jne .good
    mov ecx, [es:di + 12]
    jecxz .skip
.good:
    inc bp
    add di, 32
.skip:
    test ebx, ebx
    jne .e820lp
.done:
    xor eax, eax
    mov cx, 8
    rep stosd
    mov [E820_COUNT_ADDR], bp
    ret

enable_a20:
    mov si, msg_a20
    call print_string
    mov ax, 0x2401
    int 0x15
    jnc short .done
    cli
    call .wait
    mov al, 0xAD
    out 0x64, al
    call .wait
    mov al, 0xD0
    out 0x64, al
    call .wait2
    in al, 0x60
    push ax
    call .wait
    mov al, 0xD1
    out 0x64, al
    call .wait
    pop ax
    or al, 2
    out 0x60, al
    call .wait
    mov al, 0xAE
    out 0x64, al
    call .wait
    sti
.done:
    ret
.wait:
    in al, 0x64
    test al, 2
    jnz .wait
    ret
.wait2:
    in al, 0x64
    test al, 1
    jz .wait2
    ret

load_stage2:
    mov ah, 0x42
    mov dl, [BOOT_DRIVE_ADDR]
    mov si, dap
    int 0x13
    ret

print_string:
    lodsb
    or al, al
    jz short .done
    mov ah, 0x0E
    int 0x10
    jmp short print_string
.done:
    ret

disk_error:
    mov si, msg_derr
    call print_string
    jmp short $

msg_boot:  db 'BasilicaOS Boot...', 13, 10, 0
msg_e820:  db 'E820 ', 0
msg_a20:   db 'A20 ', 0
msg_load:  db 'Stage2 ', 0
msg_derr:  db 'DiskErr!', 0

align 4
dap:
    db 0x10, 0x00
    dw STAGE2_SECTORS
    dw STAGE2_ADDR
    dw 0x0000
    dq STAGE2_LBA

times 510-($-$$) db 0
dw 0xAA55
