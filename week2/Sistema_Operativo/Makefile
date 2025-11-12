CC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy
ASM = nasm

CFLAGS = -m32 -ffreestanding -fno-pic -nostdlib -Os -g -ffunction-sections -fdata-sections -Ikernel_boot
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib --gc-sections
ASMFLAGS = -f bin

# Tamaño por defecto de la imagen (puedes sobrescribir con `make IMAGE_SIZE=10M`)
IMAGE_SIZE ?= 1440k

# Convertidor portable de sufijos (k, M, G, T) a bytes.
# Se reutiliza en las recetas para no repetir el awk largo.
AWK_TO_BYTES = awk -v s="$(IMAGE_SIZE)" 'BEGIN{if(s==""){print 0;exit} l=tolower(substr(s,length(s),1)); if(l=="k"){n=substr(s,1,length(s)-1);print int(n*1024)} else if(l=="m"){n=substr(s,1,length(s)-1);print int(n*1024*1024)} else if(l=="g"){n=substr(s,1,length(s)-1);print int(n*1024*1024*1024)} else if(l=="t"){n=substr(s,1,length(s)-1);print int(n*1024*1024*1024*1024)} else {print int(s)} }'

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
start.o: kernel_boot/start.S
	$(CC) $(CFLAGS) -c kernel_boot/start.S -o start.o

kernel_deps = kernel_boot/kernel.c kernel_boot/kernel.h kernel_boot/kernelfunciones.c kernel_boot/module_info.h kernel_boot/elf.h

kernel.o: $(kernel_deps)
	$(CC) $(CFLAGS) -c kernel_boot/kernel.c -o kernel.o

kernelfunciones.o: kernel_boot/kernelfunciones.c kernel_boot/kernel.h kernel_boot/module_info.h kernel_boot/elf.h
	$(CC) $(CFLAGS) -c kernel_boot/kernelfunciones.c -o kernelfunciones.o

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

# === KERNEL PADDING A MÚLTIPLO DE 512 ===
kernel_padded.bin: kernel.bin kernel_sects.inc
	cp kernel.bin kernel_padded.bin
	@size=$$(stat -f%z kernel.bin 2>/dev/null || stat -c%s kernel.bin); \
	sectors=$$((($$size + 511) / 512)); \
	pad=$$((sectors * 512)); \
	truncate -s $$pad kernel_padded.bin; \
	echo "PseudoKernel padded a $$pad bytes ($$sectors sectores)"

# === IMAGEN FINAL DEL DISCO ===
# === MÓDULO POST (C + header ASM, 32-bit) ===
module_entry.o: kernel_post/module_entry.c
	$(CC) $(CFLAGS) -c kernel_post/module_entry.c -o module_entry.o

modlib.o: kernel_post/modlib.c kernel_post/modlib.h
	$(CC) $(CFLAGS) -c kernel_post/modlib.c -o modlib.o

module_post.elf: module_entry.o modlib.o kernel_post/module.ld
	$(LD) -m elf_i386 -T kernel_post/module.ld -nostdlib -o module_post.elf module_entry.o modlib.o
	@echo "Modulo ELF: $$(stat -f%z module_post.elf 2>/dev/null || stat -c%s module_post.elf) bytes"

module_post.img: module_post.elf
	@size=$$(stat -f%z module_post.elf 2>/dev/null || stat -c%s module_post.elf); \
	sectors=$$((($$size + 511) / 512)); \
	cp module_post.elf module_post.img; \
	truncate -s $$((sectors * 512)) module_post.img; \
	echo "Modulo ELF padded: $$size bytes -> $$sectors sectores"; \
	echo "#ifndef MODULE_INFO_H" > kernel_boot/module_info.h; \
	echo "#define MODULE_INFO_H" >> kernel_boot/module_info.h; \
	echo "#define MODULE_SIZE_BYTES $$size" >> kernel_boot/module_info.h; \
	echo "#define MODULE_SECTORS_COUNT $$sectors" >> kernel_boot/module_info.h; \
	echo "#endif" >> kernel_boot/module_info.h

kernel_boot/module_info.h: module_post.img
	@true


# === TRUNCAMOS POR SEGURIDAD, PERO AL TAMAÑO DE LA IMAGEN PARA INCLUIR TODO ===


os-image.bin: stage1.bin stage2.bin kernel_padded.bin module_post.img
	@echo "Concatenando imagen..."
	cat stage1.bin stage2.bin kernel_padded.bin module_post.img > os-image.bin
	actual=$$(stat -f%z os-image.bin 2>/dev/null || stat -c%s os-image.bin); \
	desired=$$($(AWK_TO_BYTES)); \
	hr_actual=$$(awk -v b=$$actual 'BEGIN{u[1]="B";u[2]="KB";u[3]="MB";u[4]="GB";i=1; while(b>=1024 && i<4){b=b/1024;i++} printf "%.2f %s", b, u[i]}'); \
	hr_desired=$$(awk -v b=$$desired 'BEGIN{u[1]="B";u[2]="KB";u[3]="MB";u[4]="GB";i=1; while(b>=1024 && i<4){b=b/1024;i++} printf "%.2f %s", b, u[i]}'); \
	if [ "$$actual" -gt "$$desired" ]; then \
		echo "Imagen concatenada: $$actual bytes ($$hr_actual)  >  IMAGE_SIZE = $(IMAGE_SIZE) ($$desired bytes, $$hr_desired). Ampliando imagen a $$actual bytes."; \
		truncate -s $$actual os-image.bin; \
	else \
		echo "Imagen final: $$desired bytes ($$hr_desired) (IMAGE_SIZE = $(IMAGE_SIZE))."; \
		truncate -s $$desired os-image.bin; \
	fi
	@echo "=== Layout del disco ==="
	@echo "Sector 1:     Stage1 (512 bytes)"
	@echo "Sectores 2-3: Stage2 (1024 bytes)"
	@echo "Sectores 4-?: Kernel ($$(stat -f%z kernel_padded.bin 2>/dev/null || stat -c%s kernel_padded.bin) bytes)"
	@echo "Ultimo sector kernel +1: module_post.img ($$(stat -f%z module_post.img 2>/dev/null || stat -c%s module_post.img) bytes)"

# === EJECUTAR ===
run: os-image.bin
	qemu-system-i386 -no-reboot -drive format=raw,file=os-image.bin

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
	actual=$$(stat -f%z test-stage1.bin 2>/dev/null || stat -c%s test-stage1.bin); \
	desired=$$($(AWK_TO_BYTES)); \
	hr_actual=$$(awk -v b=$$actual 'BEGIN{u[1]="B";u[2]="KB";u[3]="MB";u[4]="GB";i=1; while(b>=1024 && i<4){b=b/1024;i++} printf "%.2f %s", b, u[i]}'); \
	hr_desired=$$(awk -v b=$$desired 'BEGIN{u[1]="B";u[2]="KB";u[3]="MB";u[4]="GB";i=1; while(b>=1024 && i<4){b=b/1024;i++} printf "%.2f %s", b, u[i]}'); \
	if [ "$$actual" -gt "$$desired" ]; then \
		echo "test-stage1: archivo $$actual bytes ($$hr_actual)  >  IMAGE_SIZE = $(IMAGE_SIZE) ($$desired bytes, $$hr_desired). Ampliando a $$actual bytes."; \
		truncate -s $$actual test-stage1.bin; \
	else \
		echo "test-stage1: imagen final: $$desired bytes ($$hr_desired) (IMAGE_SIZE = $(IMAGE_SIZE))."; \
		truncate -s $$desired test-stage1.bin; \
	fi
	qemu-system-i386 -drive format=raw,file=test-stage1.bin

test-stage2: stage1.bin stage2.bin
	cat stage1.bin stage2.bin > test-stage2.bin
	actual=$$(stat -f%z test-stage2.bin 2>/dev/null || stat -c%s test-stage2.bin); \
	desired=$$($(AWK_TO_BYTES)); \
	hr_actual=$$(awk -v b=$$actual 'BEGIN{u[1]="B";u[2]="KB";u[3]="MB";u[4]="GB";i=1; while(b>=1024 && i<4){b=b/1024;i++} printf "%.2f %s", b, u[i]}'); \
	hr_desired=$$(awk -v b=$$desired 'BEGIN{u[1]="B";u[2]="KB";u[3]="MB";u[4]="GB";i=1; while(b>=1024 && i<4){b=b/1024;i++} printf "%.2f %s", b, u[i]}'); \
	if [ "$$actual" -gt "$$desired" ]; then \
		echo "test-stage2: archivo $$actual bytes ($$hr_actual)  >  IMAGE_SIZE = $(IMAGE_SIZE) ($$desired bytes, $$hr_desired). Ampliando a $$actual bytes."; \
		truncate -s $$actual test-stage2.bin; \
	else \
		echo "test-stage2: imagen final: $$desired bytes ($$hr_desired) (IMAGE_SIZE = $(IMAGE_SIZE))."; \
		truncate -s $$desired test-stage2.bin; \
	fi
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
	rm -f *.bin *.o *.elf kernel_sects.inc test-*.bin module_post.img module_post.elf kernel_boot/module_info.h os-image.bin

.PHONY: all clean run test-stage1 test-stage2 info
