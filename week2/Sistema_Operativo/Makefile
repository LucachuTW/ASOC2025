CC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy
ASM = nasm

CFLAGS = -m32 -ffreestanding -fno-pic -nostdlib -O0 -g -Ikernel
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib
ASMFLAGS = -f bin

.PHONY: all clean run test-stage1 test-stage2 info debug

all: os-image.bin

# === STAGE 1 BOOTLOADER (512 bytes) ===
stage1.bin: boot/stage1.asm
	$(ASM) $(ASMFLAGS) boot/stage1.asm -o stage1.bin
	@size=$$(stat -f%z stage1.bin 2>/dev/null || stat -c%s stage1.bin); \
	if [ "$$size" != "512" ]; then \
		echo "ERROR: stage1.bin es $$size bytes (debe ser 512)"; \
		exit 1; \
	fi

# === KERNEL ===
start.o: kernel/start.S
	$(CC) $(CFLAGS) -c kernel/start.S -o start.o

kernel.o: kernel/kernel.c kernel/kernel.h kernel/kernelfunciones.c
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel.o

kernelfunciones.o: kernel/kernelfunciones.c kernel/kernel.h
	$(CC) $(CFLAGS) -c kernel/kernelfunciones.c -o kernelfunciones.o

kernel.elf: start.o kernel.o kernelfunciones.o linker.ld
	$(LD) $(LDFLAGS) -o kernel.elf start.o kernel.o kernelfunciones.o

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary kernel.elf kernel.bin
	@echo "Kernel compilado: $$(stat -f%z kernel.bin 2>/dev/null || stat -c%s kernel.bin) bytes"

# === GENERAR kernel_sects.inc ===
kernel_sects.inc: kernel.bin
	@size=$$(stat -f%z kernel.bin 2>/dev/null || stat -c%s kernel.bin); \
	sectors=$$((($$size + 511) / 512)); \
	echo "; Auto-generado" > kernel_sects.inc; \
	echo "KS_COUNT equ $$sectors" >> kernel_sects.inc; \
	echo "Kernel: $$size bytes = $$sectors sectores"

# === STAGE 2 BOOTLOADER (1024 bytes) ===
stage2.bin: boot/stage2.asm kernel_sects.inc
	$(ASM) $(ASMFLAGS) boot/stage2.asm -o stage2.bin
	@size=$$(stat -f%z stage2.bin 2>/dev/null || stat -c%s stage2.bin); \
	if [ "$$size" != "1024" ]; then \
		echo "ERROR: stage2.bin es $$size bytes (debe ser 1024)"; \
		exit 1; \
	fi

# === IMAGEN FINAL DEL DISCO ===
os-image.bin: stage1.bin stage2.bin kernel.bin
	cat stage1.bin stage2.bin kernel.bin > os-image.bin
	truncate -s 1440k os-image.bin
	@echo "=== Layout del disco ==="
	@echo "Sector 0:     Stage1 (512 bytes)"
	@echo "Sectores 1-2: Stage2 (1024 bytes)"
	@echo "Sector 3+:    Kernel ($$(stat -f%z kernel.bin 2>/dev/null || stat -c%s kernel.bin) bytes)"

# === EJECUTAR ===
run: os-image.bin
	qemu-system-i386 -drive format=raw,file=os-image.bin

# === RECONSTRUCCIÓN CON TRACES DE MAKE ===
debug:
	@echo "=== Debug: reconstrucción verbosa ==="
	$(MAKE) clean
	$(MAKE) --debug=v --always-make VERBOSE=1 os-image.bin
	@echo "=== Estado tras debug build ==="
	$(MAKE) info

# === TESTS INCREMENTALES ===
test-stage1: stage1.bin
	cp stage1.bin test-stage1.bin
	truncate -s 1440k test-stage1.bin
	qemu-system-i386 -drive format=raw,file=test-stage1.bin

test-stage2: stage1.bin stage2.bin
	cat stage1.bin stage2.bin > test-stage2.bin
	truncate -s 1440k test-stage2.bin
	qemu-system-i386 -drive format=raw,file=test-stage2.bin

# === INFO ===
info:
	@echo "=== Estado ==="
	@ls -lh stage1.bin stage2.bin kernel.bin os-image.bin 2>/dev/null || echo "Archivos faltantes"
	@echo ""
	@echo "=== kernel_sects.inc ==="
	@cat kernel_sects.inc 2>/dev/null || echo "No generado"

# === LIMPIAR ===
clean:
	rm -f *.bin *.o *.elf kernel_sects.inc test-*.bin

.PHONY: all clean run test-stage1 test-stage2 info
