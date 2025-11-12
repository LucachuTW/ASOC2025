OUTPUT_FORMAT(elf32-i386)
ENTRY(module_entry)

SECTIONS {
  /* El módulo se vincula para ejecutarse en 0x00120000 (como antes, pero ahora se describe vía ELF) */
  . = 0x00120000;

  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.eh_frame) *(.comment) }

  __module_end = .;
  __module_size = __module_end - 0x00120000;
}
