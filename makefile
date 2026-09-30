CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-gcc

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
LDFLAGS = -ffreestanding -O2 -nostdlib

OBJS = boot.o gdt.o gdt_setup.o kernel.o io.o pic.o idt.o idt_setup.o terminal.o idt_handlers.o string.o kprintf.o pmm.o vmm.o heap.o pci.o 

myos.iso: myos 
	mkdir -p isodir/boot/grub
	cp myos isodir/boot/myos
	grub-mkrescue -o myos.iso isodir

myos: $(OBJS) linker.ld
	$(LD) -T linker.ld -o myos $(LDFLAGS) $(OBJS) -lgcc

boot.o: boot.s
	$(AS) boot.s -o boot.o

gdt_setup.o: gdt_setup.s
	$(AS) gdt_setup.s -o gdt_setup.o

gdt.o: gdt.c
	$(CC) $(CFLAGS) -c gdt.c -o gdt.o

io.o: io.c
	$(CC) $(CFLAGS) -c io.c -o io.o

pic.o: pic.c
	$(CC) $(CFLAGS) -c pic.c -o pic.o

idt.o: idt.c
	$(CC) $(CFLAGS) -c idt.c -o idt.o

idt_setup.o: idt_setup.s
	$(AS) idt_setup.s -o idt_setup.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

pmm.o: pmm.c
	$(CC) $(CFLAGS) -c pmm.c -o pmm.o

terminal.o: terminal.c
	$(CC) $(CFLAGS) -c terminal.c -o terminal.o

idt_handlers.o: idt_handlers.c
	$(CC) $(CFLAGS) -c idt_handlers.c -o idt_handlers.o

string.o: string.c
	$(CC) $(CFLAGS) -c string.c -o string.o

kprint.o: kprintf.c
	$(CC) $(CFLAGS) -c kprintf.c -o kprintf.o

vmm.o: vmm.c
	$(CC) $(CFLAGS) -c vmm.c -o vmm.o

heap.o: heap.c
	$(CC) $(CFLAGS) -c heap.c -o heap.o

pci.o: pci.c
	$(CC) $(CFLAGS) -c pci.c -o pci.o

clean:
	rm -f *.o myos myos.iso
