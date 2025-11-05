all: os-image.bin

boot.bin: boot/boot.asm
	nasm -f bin $< -o $@

os-image.bin: boot.bin
	cp $< $@

run: os-image.bin
	qemu-system-x86_64 -drive format=raw,file=$<

clean:
	rm -f *.bin *.iso
