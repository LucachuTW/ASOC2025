.code32
.section .text
.global _start

_start:
    cli
    
    # Configurar stack lejos del kernel
    mov $0x90000, %esp
    mov %esp, %ebp
    
    # Llamar a kmain
    call kmain
    
    # Si retorna, colgar
1:
    hlt
    jmp 1b