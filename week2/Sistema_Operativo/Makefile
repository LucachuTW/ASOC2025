CC = x86_64-elf-gcc           # Compilador cross-compiler para i386
LD = x86_64-elf-ld            # Enlazador
OBJCOPY = x86_64-elf-objcopy  # Extractor de binarios

all: os-image.bin             # Target por defecto

# 1. Ensamblar bootloader
boot.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o boot.bin
	# -f bin: formato binario plano (no ELF)

# 2. Compilar entry point en assembly
start.o: kernel/start.S
	$(CC) -m32 -c kernel/start.S -o start.o
	# -m32: 32-bit mode
	# -c: compilar sin enlazar

# 3. Compilar kernel C
kernel.o: kernel/kernel.c kernel/kernel.h
	$(CC) -m32 -ffreestanding -fno-pic -fomit-frame-pointer -nostdlib -Ikernel -c kernel/kernel.c -o kernel.o
	# -ffreestanding: sin biblioteca estándar (no hay printf, malloc, etc.)
	# -fno-pic: código no independiente de posición (más simple)
	# -fomit-frame-pointer: no usar EBP como frame pointer (optimización)
	# -nostdlib: no enlazar con libc
	# -Ikernel: buscar headers en ./kernel/

# 4. Enlazar kernel
kernel.bin: start.o kernel.o linker.ld
	$(LD) -m elf_i386 -T linker.ld -nostdlib -o kernel.elf start.o kernel.o
	# -T linker.ld: usar script personalizado
	# Orden importante: start.o primero (contiene _start)
	
	$(OBJCOPY) -O binary kernel.elf kernel.bin
	# Extrae solo el código binario (sin headers ELF)
	
	@echo "Kernel:" $$(stat -f%z kernel.bin) "bytes," $$(((($$(stat -f%z kernel.bin)+511)/512))) "sectores"

# 5. Crear imagen de disco
os-image.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.bin
	# Concatenar: boot sector (512B) + kernel
	
	dd if=/dev/zero bs=512 count=10 >> os-image.bin 2>/dev/null
	# Añadir 5KB de padding (para simular disco)

run: os-image.bin
	qemu-system-x86_64 -drive format=raw,file=os-image.bin

clean:
	rm -f *.bin *.o *.elf