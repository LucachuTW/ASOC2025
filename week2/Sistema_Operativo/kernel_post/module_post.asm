; Módulo post (32-bit) con cabecera y punto de entrada
; Formato:
; 0x00: 'MOD0' (4 bytes)
; 0x04: version (u16)
; 0x06: length  (u16) - opcional
; 0x08: entry_off (u32) -> offset relativo al inicio del módulo
; 0x0C: reservado / mensaje

BITS 32
ORG 0

magic:      db 'MOD0'
version:    dw 0x0001
length:     dw 16
entry_off:  dd module_main - $$
msg:        db 'Modulo POST ejecutandose',0

; ==== Rutinas auxiliares simples (VGA texto) ====
%define VGA_MEM 0xB8000
%define VGA_W   80
%define VGA_H   25

; void cls(attr=0x07)
cls:
	push eax
	push ecx
	push edi
	mov eax, 0x07200720       ; dos celdas: ' ' 0x20 con attr 0x07
	mov ecx, (VGA_W*VGA_H)/2  ; escribimos de 2 en 2 celdas
	mov edi, VGA_MEM
	rep stosd
	pop edi
	pop ecx
	pop eax
	ret

; void putc_xy(char c, int x, int y, byte attr)
; [esp+4]=c [esp+8]=x [esp+12]=y [esp+16]=attr
putc_xy:
	push eax
	push ebx
	push ecx
	push edx
	mov al, [esp+20]
	mov ebx, [esp+24]
	mov ecx, [esp+28]
	mov dl, [esp+32]
	; offset = (y*80 + x) * 2
	mov eax, ecx
	imul eax, eax, VGA_W
	add eax, ebx
	shl eax, 1
	add eax, VGA_MEM
	mov [eax], al
	mov [eax+1], dl
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

; void print_xy(char* s, int x, int y, byte attr)
print_xy:
	push ebp
	mov ebp, esp
	push esi
	mov esi, [ebp+8]
	mov ebx, [ebp+12]
	mov ecx, [ebp+16]
	mov dl,  [ebp+20]
.loop:
	lodsb
	test al, al
	jz .done
	push edx
	push ecx
	push ebx
	push eax
	call putc_xy
	inc ebx
	jmp .loop
.done:
	pop esi
	mov esp, ebp
	pop ebp
	ret

; ==== Entrada del módulo ====
module_main:
	; Limpiar pantalla
	call cls
	; Título
	push 0x0B
	push 2
	push 2
	push title
	call print_xy
	add esp, 16
	; Barra de carga con '='
	mov ecx, 40           ; ancho
	mov ebx, 20           ; x inicial
	mov edx, 8            ; y
.bar_loop:
	; dibujar '=' en (x+iter, y)
	mov eax, ebx
	mov esi, 40
	sub esi, ecx
	add eax, esi   ; posición actual
	push 0x0A        ; verde
	push edx
	push eax
	push '='
	call putc_xy
	add esp, 16
	; pequeño retardo
	mov edi, 200000
.delay:
	dec edi
	jnz .delay
	loop .bar_loop
	; Mensaje final
	push 0x0A
	push 26
	push 12
	push done
	call print_xy
	add esp, 16
	ret

title: db 'MODULO: Barra de carga',0
done:  db 'Modulo finalizado. Volviendo al kernel...',0

; Relleno al sector
TIMES 512-($-$$) db 0
