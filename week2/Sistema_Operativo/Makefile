CC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy
CFLAGS = -m32 -ffreestanding -fno-pic -fomit-frame-pointer -nostdlib -std=c11 -O0 -g -Ikernel
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

KERNEL_SECTORS = $(shell test -f kernel.bin && echo $$((($$(stat -f%z kernel.bin)+511)/512)) || echo 1)

.PHONY: all run debug clean

all: os-image.bin

boot.bin: boot/boot.asm kernel.bin
	nasm -f bin -D KERNEL_SECTORS=$(KERNEL_SECTORS) boot/boot.asm -o boot.bin

start.o: kernel/start.S
	$(CC) -m32 -g -c kernel/start.S -o start.o

kernel.o: kernel/kernel.c kernel/kernel.h
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel.o

kernel.elf: start.o kernel.o linker.ld
	$(LD) $(LDFLAGS) -o kernel.elf start.o kernel.o

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary kernel.elf kernel.bin
	@echo "Kernel:" $$(stat -f%z kernel.bin) "bytes," $(KERNEL_SECTORS) "sectores"

os-image.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.bin
	truncate -s 1440k os-image.bin

run: os-image.bin
	qemu-system-i386 -fda os-image.bin

debug: os-image.bin
	qemu-system-i386 -drive format=raw,file=os-image.bin -no-reboot -d int,cpu_reset

clean:
	rm -f *.bin *.o *.elf