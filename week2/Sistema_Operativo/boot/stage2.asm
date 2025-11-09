[BITS 16]
[ORG 0x7E00]

%include "kernel_sects.inc"

; ===========================================
; STAGE 2 - BOOTLOADER REESCRITO
; ===========================================
; Tareas:
; 1. Cargar kernel desde disco (sector 4+)
; 2. Habilitar línea A20
; 3. Configurar GDT
; 4. Cambiar a modo protegido 32-bit
; 5. Saltar al kernel en 0x10000
; ===========================================

start:
    ; Limpiar segmentos
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Guardar número de disco (viene en DL desde stage1)
    mov [boot_drive], dl
    
    ; Mensaje inicial
    mov si, msg_stage2
    call print

    ; ===========================================
    ; PASO 1: CARGAR KERNEL DESDE DISCO
    ; ===========================================
    ; Calculamos dónde empieza el kernel:
    ; - Sector 1: stage1 (512 bytes = 1 sector)
    ; - Sectores 2-3: stage2 (1024 bytes = 2 sectores)
    ; - Sector 4+: kernel (KS_COUNT sectores)
    
    mov si, msg_load
    call print
    
    ; Configurar destino: 0x1000:0x0000 = dirección lineal 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    ; Configurar lectura desde disco
    mov ah, 0x02            ; INT 13h función 02h = leer sectores
    mov al, KS_COUNT        ; Número de sectores (desde kernel_sects.inc)
    mov ch, 0               ; Cilindro 0
    mov cl, 4               ; Sector 4 (donde empieza el kernel)
    mov dh, 0               ; Cabeza 0
    mov dl, [boot_drive]    ; Disco de arranque
    
    int 0x13
    jc .error
    
    mov si, msg_ok
    call print
    
    ; Mostrar primer byte del kernel cargado (debug)
    mov si, msg_first_byte
    call print
    mov ax, 0x1000
    mov es, ax
    mov al, [es:0]
    call print_hex8
    call print_newline
    
    jmp .continue

.error:
    mov si, msg_error
    call print
    hlt
    jmp $

.continue:
    ; ===========================================
    ; PASO 2: HABILITAR LÍNEA A20
    ; ===========================================
    call enable_a20
    
    ; ===========================================
    ; PASO 3: CARGAR GDT Y CAMBIAR A MODO PROTEGIDO
    ; ===========================================
    cli                     ; Deshabilitar interrupciones
    lgdt [gdt_descriptor]   ; Cargar tabla GDT
    
    ; Activar bit PE (Protection Enable) en CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump para limpiar pipeline y cargar CS con selector de código
    jmp 0x08:protected_mode

; ===========================================
; FUNCIONES DE MODO REAL (16-BIT)
; ===========================================

; Habilitar A20 usando método Fast A20
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; Imprimir string (SI apunta al string, terminado en 0)
print:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Imprimir salto de línea
print_newline:
    pusha
    mov si, msg_newline
    call print
    popa
    ret

; Imprimir byte en hexadecimal (AL = byte)
print_hex8:
    pusha
    mov ah, al
    shr al, 4
    call .print_nibble
    mov al, ah
    and al, 0x0F
    call .print_nibble
    popa
    ret
.print_nibble:
    cmp al, 9
    jg .letter
    add al, '0'
    jmp .print
.letter:
    add al, 'A' - 10
.print:
    mov ah, 0x0E
    int 0x10
    ret

; ===========================================
; DATOS
; ===========================================
boot_drive          db 0

msg_stage2          db '[Stage2] Iniciado', 13, 10, 0
msg_load            db '[Stage2] Cargando kernel...', 0
msg_ok              db ' OK', 13, 10, 0
msg_error           db ' ERROR!', 13, 10, 0
msg_first_byte      db '[Stage2] Primer byte del kernel: 0x', 0
msg_newline         db 13, 10, 0

; ===========================================
; GDT (Global Descriptor Table)
; ===========================================
align 8
gdt_start:

gdt_null:           ; Descriptor nulo (obligatorio)
    dq 0x0

gdt_code:           ; Descriptor de segmento de código
    dw 0xFFFF       ; Límite (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Flags: presente, ring 0, código, ejecutable, legible
    db 11001111b    ; Flags: 4KB granular, 32-bit, límite (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_data:           ; Descriptor de segmento de datos
    dw 0xFFFF       ; Límite (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10010010b    ; Flags: presente, ring 0, datos, escribible
    db 11001111b    ; Flags: 4KB granular, 32-bit, límite (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Tamaño de la GDT
    dd gdt_start        ; Dirección de la GDT (absoluta)

; ===========================================
; CÓDIGO DE MODO PROTEGIDO (32-BIT)
; ===========================================
[BITS 32]
protected_mode:
    ; Configurar todos los selectores de segmento a datos (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Configurar pila en 0x90000
    mov esp, 0x90000
    
    ; Saltar al kernel cargado en 0x10000
    jmp 0x10000

; ===========================================
; PADDING
; ===========================================
times 1024-($-$$) db 0

