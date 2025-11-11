[org 0x7C00] ; posición por convención
[bits 16] ; instrucciones limitadas a las que tenemos acceso

start:
    ; Inicializar segmentos y stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl ; guardar drive de la BIOS

    ; Banner mínimo (branding Virus Payal)
    mov si, brand1
    call puts
    mov si, brand2
    call puts

    ; NOTA: hemos eliminado la persistencia en CMOS y la variable last_drive
    ; para simplificar (menos bloat). Se usa directamente el valor que BIOS
    ; nos pasa en DL (guardado en boot_drive arriba).

.scan:
    ; Sondeo simple de 4 unidades con índice legible; construimos máscara de presencia
    xor di, di
    mov byte [present_mask], 0
.probe:
    ; Imprimir índice [0-3]
    mov al, '['
    call putc
    mov ax, di
    add al, '0'
    call putc
    mov al, ']'
    call putc
    mov al, ' '
    call putc
    
    ; Probar unidad
    mov bl, [drives + di]
    mov dl, bl
    xor ah, ah
    int 0x13            ; reset
    jc .not_found
    
    ; Disco encontrado: imprimir hex y marcar presencia en la máscara
    mov al, bl
    call puthex
    mov al, 1
    mov cx, di
    shl al, cl
    or [present_mask], al
    ; imprimir espacio separador
    mov al, ' '
    call putc
    jmp .next

.not_found:
    ; Disco no responde: imprimir -- y espacio separador
    mov al, '-'
    call putc
    call putc
    mov al, ' '
    call putc
    jmp .next
    
.next:
    mov al, ' '
    call putc
    inc di
    cmp di, 4
    jb .probe

    ; Prompt (reducido)
    mov si, m_sel
    call puts

.wait_key:
    ; Leer tecla con eco
    xor ah, ah
    int 0x16            ; AL = tecla ASCII
    
    ; Mostrar eco de la tecla (imprime el carácter ASCII que devolvió int 0x16)
    push ax
    call putc
    pop ax
    
    ; Si es ENTER, usar default (y persistir si aún no estaba)
    cmp al, 0x0D
    je .use_default
    
    ; Si es 1-3, seleccionar
    cmp al, '1'
    jb .wait_key        ; < '1': ignorar y esperar otra tecla
    cmp al, '3'
    ja .wait_key        ; > '3': ignorar y esperar otra tecla
    
    ; Tecla válida preliminar: convertir índice
    sub al, '0'
    xor bh, bh
    mov bl, al
    ; Validar que la unidad está presente según máscara
    mov al, 1
    mov cx, bx
    shl al, cl
    and al, [present_mask]
    jz .invalid_sel      ; no presente -> pedir otra tecla
    ; Unidad presente: aplicar selección
    mov dl, [drives + bx]
    mov [boot_drive], dl
    ; Guardamos la selección en la zona compartida para stage2
    mov di, 0x7E04
    mov [di], dl
    jmp .go

.invalid_sel:
    ; Aviso breve con '!'
    mov al, '!'
    call putc
    jmp .wait_key

.use_default:
    ; ENTER presionado: usar boot_drive actual y guardarlo en zona compartida
    mov dl, [boot_drive]
    mov di, 0x7E04
    mov [di], dl

.go:
    ; Cargar stage2
    mov bx, 0x7E00
    mov dh, 2
    mov cl, 2
    call disk_load
    jmp 0x0000:0x7E00

; Rutinas compactas
cmos_read: ; (dejado como stub por compatibilidad futura)
    ; Antes esta rutina leía CMOS en AL y guardaba en BL. Ya no la usamos,
    ; pero dejamos un stub que devuelve 0 en BL para evitar cambios mayores
    xor al, al
    mov bl, al
    ret

cmos_write: ; (stub, no hacemos writes a CMOS para reducir bloat)
    ret

disk_load:
    push dx
    mov ah, 0x02
    mov al, dh
    mov ch, 0
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc $
    pop dx
    ret

puts:
    lodsb
    test al, al
    jz .d
    mov ah, 0x0E
    int 0x10
    jmp puts
.d: ret

putc:
    mov ah, 0x0E
    int 0x10
    ret

puthex:
    push ax
    shr al, 4
    call .n
    pop ax
    and al, 0x0F
.n: cmp al, 10
    jl .d
    add al, 7
.d: add al, '0'
    call putc
    ret

boot_drive: db 0 
drives: db 0x00,0x80,0x01,0x81 ;Hardcodeado porque son las direcciones escogidas
m_sel: db 13,10,'1-3/ENTER:',0
present_mask: db 0
brand1: db '[Virus Payal]',13,10,0
brand2: db 'Selector discos:',13,10,0

; Rellenar el resto del sector con ceros hasta la firma
times 510 - ($ - $$) db 0
dw 0xAA55