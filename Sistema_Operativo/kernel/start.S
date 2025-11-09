.section .text
.global _start
.extern kmain

_start:
    cli
    mov $0x90000, %esp
    mov %esp, %ebp
    
    # DEBUG: escribir '+' antes de llamar a kmain
    movb $'+', 0xB8004
    movb $0x0F, 0xB8005
    
    call kmain
hang:
    jmp hang