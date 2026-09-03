# BasilicaOS Makefile (64-bit)

CC      := $(shell which x86_64-elf-gcc 2>/dev/null || echo gcc)
LD      := $(shell which x86_64-elf-ld 2>/dev/null || echo ld)
ASM     := nasm

CFLAGS  := -ffreestanding -O2 -Wall -Wextra -std=gnu99 -mno-red-zone \
           -mno-mmx -mno-sse -mno-sse2 -fno-pie -fno-stack-protector
LDFLAGS := -nostdlib -T kernel/linker.ld -z max-page-size=0x1000 -no-pie
AFLAGS  := -f elf64

all: basilica.img

# For bootloader
boot/stage1.bin: boot/stage1.asm
	$(ASM) -f bin $< -o $@

boot/stage2.bin: boot/stage2.asm
	$(ASM) -f bin $< -o $@

# kernel stuff!
kernel/entry.o: kernel/entry.asm
	$(ASM) $(AFLAGS) $< -o $@

kernel/isr.o: kernel/isr.asm
	$(ASM) $(AFLAGS) $< -o $@

kernel/kernel.o: kernel/kernel.c kernel/basilica.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/shell/console.o: kernel/shell/console.c kernel/basilica.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/mem.o: kernel/mem.c kernel/basilica.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/shell/interrupts.o: kernel/shell/interrupts.c kernel/basilica.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/shell/keyboard.o: kernel/shell/keyboard.c kernel/basilica.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/shell/shell.o: kernel/shell/shell.c kernel/basilica.h
	$(CC) $(CFLAGS) -c $< -o $@


kernel.bin: kernel/entry.o kernel/isr.o kernel/kernel.o kernel/shell/console.o kernel/mem.o kernel/shell/interrupts.o kernel/shell/keyboard.o kernel/shell/shell.o
	$(LD) $(LDFLAGS) -o kernel.elf $^
	objcopy -O binary kernel.elf $@


basilica.img: boot/stage1.bin boot/stage2.bin kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=32768 2>/dev/null
	dd if=boot/stage1.bin of=$@ conv=notrunc bs=512 seek=0
	dd if=boot/stage2.bin of=$@ conv=notrunc bs=512 seek=1
	dd if=kernel.bin of=$@ conv=notrunc bs=512 seek=65

run: basilica.img
	qemu-system-x86_64 -drive format=raw,file=basilica.img \
		-serial stdio -m 128

debug: basilica.img
	qemu-system-x86_64 -drive format=raw,file=basilica.img \
		-serial stdio -s -S -m 128

clean:
	rm -f kernel.bin basilica.img kernel.elf
	rm -f boot/*.bin kernel/*.o /kernel/shell/*.o

.PHONY: all run debug clean
