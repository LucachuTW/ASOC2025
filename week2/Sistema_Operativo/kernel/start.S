.section .text
.global _start
.extern kmain

_start:
    cli                   # 1. Deshabilitar interrupciones
    mov $0x80000, %esp    # 2. Configurar stack en 0x80000 (576KB)
    mov %esp, %ebp        # 3. Base pointer = top del stack
    
    # 4. Debug visual (opcional)
    movb $'+', 0xB8004
    movb $0x0F, 0xB8005
    
    call kmain            
hang:
    jmp hang              # 5. Si kmain retorna, colgar