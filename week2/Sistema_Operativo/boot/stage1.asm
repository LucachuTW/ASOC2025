[org 0x7C00] ; posición por convención
[bits 16] ; instrucciones limitadas a las que tenemos acceso

start:
    ; Inicializar segmentos y stack a 0 pues no conocemos su estado previo
    xor ax, ax ; pone ax a 0 empleando menos bits que haciendo mv
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Guardar el drive de arranque que nos pasa la BIOS en DL
    mov [boot_drive], dl

    
    mov si, msg_loading ; Mensaje de inicio, este texto se verá exclusivamente si hay algún error y no conseguimos cargar el stage 2
    call print_string 

    ; Cargar stage2 (2 sectores) a la dirección 0x7E00
    mov bx, 0x7E00      ; Dirección de carga para stage2, probando a cargar otra crashea en vez de dejar ver el mensaje, dado que leer fuera del rango de 16 bits es imposible y da errores
    mov dh, 2           ; Número de sectores a leer
    mov cl, 2           ; Sector de inicio (el 1 es este mismo bootloader)
    call disk_load

    ; Mensaje de éxito
    mov si, msg_success
    call print_string

    ; Saltar a stage2 desde cs valiendo 0 y usa el pipeline para limpiar instrucciones de 16 bits antes de una de 32
    jmp 0x0000:0x7E00

; --- Funciones ---

disk_load:
    push dx             ; Guardar DX (contiene número de sectores)
    mov ah, 0x02        ; Función de lectura de la BIOS
    mov al, dh          ; Número de sectores
    mov ch, 0           ; Cilindro 0
    ; CL ya tiene el sector de inicio
    mov dh, 0           ; Cabeza 0
    mov dl, [boot_drive] ; Usar el drive de arranque guardado
    ; ES:BX ya apunta al buffer de destino
    int 0x13
    jc disk_error       ; Si hay error (carry flag = 1), saltar
    pop dx
    cmp al, dh          ; Comprobar si se leyeron los sectores esperados
    jne disk_error
    ret

disk_error:
    mov si, msg_error
    call print_string
    jmp $               ; Bucle infinito

print_string:
    pusha
.loop:
    lodsb               ; Cargar byte de [SI] en AL, e incrementar SI
    cmp al, 0
    je .done
    mov ah, 0x0E        ; Función de teletipo de la BIOS
    int 0x10
    jmp .loop
.done:
    popa
    ret

; --- Datos y relleno ---

boot_drive: db 0
msg_loading: db '[S1] Cargando Stage 2...', 13, 10, 0
msg_success: db '[S1] OK', 13, 10, 0
msg_error:   db '[S1] Error de disco', 13, 10, 0

; Rellenar el resto del sector con ceros hasta la firma
times 510 - ($ - $$) db 0
dw 0xAA55