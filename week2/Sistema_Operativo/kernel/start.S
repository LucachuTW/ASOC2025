.code32
.section .text
.global _start

_start:
	cli
	
	# Configurar stack LEJOS del kernel
	mov $0x90000, %esp
	mov %esp, %ebp
	
	# Test: escribir '+' directo
	movb $'+', (0xB8004)
	movb $0x0F, (0xB8005)
	
	# Llamar a kmain
	call kmain
	
	# Si retorna, colgar
1:
	hlt
	jmp 1b