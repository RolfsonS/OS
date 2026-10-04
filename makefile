CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-gcc

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g -Iinclude
LDFLAGS = -ffreestanding -O2 -nostdlib

SRCS = arch/i386/boot/boot.s \
       arch/i386/cpu/gdt.c  arch/i386/cpu/gdt_setup.s arch/i386/cpu/idt.c arch/i386/cpu/idt_handlers.c arch/i386/cpu/idt_setup.s arch/i386/cpu/pic.c \
       arch/i386/mm/vmm.c \
       arch/i386/io.c \
       drivers/pci/pci.c drivers/video/terminal.c drivers/net/e1000.c\
       kernel/kernel.c \
       lib/kprintf.c lib/string.c \
       mm/heap.c mm/pmm.c

LINK = arch/i386/boot/linker.ld

OBJS = $(addprefix build/, $(addsuffix .o, $(basename $(SRCS))))

build/myos.iso: build/myos 
	mkdir -p build/isodir/boot/grub
	cp build/myos build/isodir/boot/myos
	cp iso/boot/grub/grub.cfg build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o build/myos.iso build/isodir

build/myos: $(OBJS) $(LINK)
	$(LD) -T $(LINK) -o build/myos $(LDFLAGS) $(OBJS) -lgcc

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $< -o $@

run: build/myos.iso
	qemu-system-i386 -cdrom build/myos.iso 

clean:
	rm -rf build

