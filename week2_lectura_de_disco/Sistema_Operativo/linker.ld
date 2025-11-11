OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
SECTIONS {
	. = 0x10000;  /* Kernel link address: 0x10000, cargado por stage2 a segmento 0x1000 */
	.text : { *(.text*) }
	.rodata : { *(.rodata*) }
	.data :  { *(.data*) }
	.bss  :  { *(.bss*) *(COMMON) }
	_end = .;
}