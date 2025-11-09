.section .module_header, "a"
.global __module_header
.extern module_entry
.extern __module_size
__module_header:
    .ascii "MBIN"         # magic estandarizado para binario plano de modulo
    .short 1              # version del formato
    .short __module_size  # length (truncado implícitamente a 16 bits por el ensamblador)
    .long module_entry - __module_header   # entry_off relativo al inicio
    .ascii "Modulo C entry"
    .byte 0
