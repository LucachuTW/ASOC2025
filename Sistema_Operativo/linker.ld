ENTRY(_start)              # Punto de entrada: primera función a ejecutar
SECTIONS {
  . = 0x8000;              # Dirección base: el kernel comienza en 0x8000
  .text : { *(.text*) }    # Sección de código ejecutable
  .rodata : { *(.rodata*) } # Datos de solo lectura (strings constantes)
  .data : { *(.data*) }    # Datos inicializados (variables globales con valor)
  .bss : { *(.bss*) *(COMMON) } # Datos no inicializados (variables sin valor inicial)
}