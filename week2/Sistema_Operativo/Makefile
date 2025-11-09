CC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy
CFLAGS = -m32 -ffreestanding -fno-pic -fomit-frame-pointer -nostdlib -std=c11 -O0 -g

all: os-image.bin

boot.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o boot.bin

start.o: kernel/start.S
	$(CC) -m32 -c kernel/start.S -o start.o

kernel.o: kernel/kernel.c kernel/kernel.h
	$(CC) $(CFLAGS) -Ikernel -c kernel/kernel.c -o kernel.o

kernel.bin: start.o kernel.o linker.ld
	$(LD) -m elf_i386 -T linker.ld -nostdlib -o kernel.elf start.o kernel.o
	$(OBJCOPY) -O binary kernel.elf kernel.bin
	@echo "Kernel:" $$(stat -f%z kernel.bin) "bytes," $$(((($$(stat -f%z kernel.bin)+511)/512))) "sectores"

os-image.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.bin
	dd if=/dev/zero bs=512 count=10 >> os-image.bin 2>/dev/null

run: os-image.bin
	qemu-system-x86_64 -drive format=raw,file=os-image.bin

debug: os-image.bin
	qemu-system-x86_64 -drive format=raw,file=os-image.bin -no-reboot -d int,cpu_reset

clean:
	rm -f *.bin *.o *.elf

.PHONY: all run debug clean