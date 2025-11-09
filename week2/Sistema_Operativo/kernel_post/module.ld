OUTPUT_FORMAT(binary)
SECTIONS {
  /* Dirección virtual donde vivirá el módulo al cargarse en RAM */
  . = 0x00012000;

  /* Colocamos la cabecera al inicio tanto en VMA como en LMA (offset 0 del fichero) */
  .module_header : { *(.module_header) }

  /* A partir de aquí, ajustamos las LMA para que el fichero sea contiguo desde 0 */
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.eh_frame) *(.comment) }

  /* Símbolo de final de imagen para que la cabecera pueda calcular el tamaño total */
  __module_end = .;
  __module_size = __module_end - 0x00012000; /* tamaño total en bytes */
}
