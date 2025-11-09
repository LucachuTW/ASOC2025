[org 0x7C00]                 ; Direccion de origen estándar
[bits 16]                      ;cuestiones de compatibilidad

cli                          ; Limpia interrupciones para que no se corrompa la memoria, porque no conocemos los estados que nos da la BIOS
xor ax, ax                   ; Iguala ax a 0 porque al cumplirse ambos es una condición falsa; además, no podemos escribir directamente a ds/es; podría ponerse un 0 en vez del segundo ax
mov ds, ax                   ; DS = 0   -> datos en segmento base 0
mov es, ax                   ; ES = 0   -> útil para copiar/cargar a memoria
mov ss, ax                   ; SS = 0   -> segmento de pila
mov sp, 0x7C00               ; SP = 0x7C00 -> top de la pila (escalamos al origen tras reiniciar las banderas de interrupciones) !!!ESTO ES EL STACK!!! Se recomienda ponerlo anterior a la posición de inicio real para tener un mayor espacio
sti                          ; Reactiva interrupciones
cld                          ; Limpia DF (aloca y opcionalmente da 6 bits de valor a cada inicializador): loadsb y stosb incrementan SI/DI en vez de decrementarlos

mov [BootDrive], dl          ; Guarda el número de disco de arranque que te da el BIOS en DL, que suele ser el bit "a la izquierda" de dx, generalmente empleado como gestor de posición

mov si, msg                  ; 6) SI apunta a la cadena terminada en 0
call print_string            ;   emplea una instrucción de la BIOS para imprimir

; === BARRA DE PROGRESO ===
mov cx, 10            ; 10 pasos

mov si, progress_msg
call print_string

.progress_loop:
    push cx
    
    mov al, '*'
    call print_char
    
    call delay
    
    pop cx
    loop .progress_loop

    mov si, done_msg
    call print_string
; === FIN BARRA ===


call load_kernel            ; Cargar kernel a 0x8000
call enable_a20             ; Para poder acceder a memoria >1Mb en modo protegido
call enter_protected_mode   ; Modo Protegido


jmp $                        ; 7) Bucle infinito (se detiene aquí)

; --- Rutinas ---

print_char:
    mov ah, 0x0E             ; Función teletipo de INT 10h (imprime AL)
    mov bh, 0x00             ; Página de vídeo
    mov bl, 0x07             ; Atributo (texto gris sobre negro)
    int 0x10
    ret

print_string:                ; Entrada: SI -> cadena terminada en 0
.next:
lodsb                    ; carga un byte de [DS:SI] en AL y hace SI++ (avanza al siguiente carácter).
test al, al              ; hace and lógico de al y al, actualiza flags porque eso es 1, pero no cambia al (al es el byte mas bajo de ax)
jz .done                 ; como saltamos la cadena si AL==0, acabamos si ZF se pone a uno por la instrucción anterior
call print_char          ;
jmp .next                ; repetimos si jz no nos saca
.done:
ret

delay:
    mov ah, 0x86
    mov cx, 0x0002    ; 150,000 microsegundos
    mov dx, 0x93E0
    int 0x15
    ret


enable_a20:
in al, 0x92             ; puerto de control
or al, 00000010b        ; set bit A20
out 0x92, al
ret

load_kernel: ; ES:BX = destino 0x0000:0x8000
xor ax, ax ; es redundante pero tengo espacio xd
mov es, ax ; lo mismo
mov bx, 0x8000 ; aquí se aloja el kernel

mov ah, 0x02        ; activo el modo de leer sectores, AL/CH/CL/DH/DL/ES:BX como parámetros
mov al, [KernelSects] ; número de sectores (ajusta)

mov ch, 0x00        ; cilindro (https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)
mov cl, 0x02        ; sector inicial (el siguiente al boot)
mov dh, 0x00        ; cabeza
mov dl, [BootDrive] ; unidad
int 0x13
jc disk_error       ; si falla -> error

ret

disk_error:
mov si, disk_msg
call print_string
jmp $

enter_protected_mode:
    cli
    lgdt [gdtr]             
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp dword 0x08:pm_start  ;  "dword" para fuerza 32-bit offset

; ---------------- GDT ----------------
gdt_start:
    ; Quiero una segmentación hasta 4gb (32-bit); La estructura de una instrucción es la siguiente:

    ;Bytes: 0-1 -> Limit[15:0]
    ;Bytes: 2-3 -> Base[15:0]
    ;Byte: 4 -> Base[23:16]
    ;Byte: 5 -> Access
    ;Byte: 6 -> Flags(4 bits altos) + Limit[19:16] (4 bits bajos)
    ;Byte: 7 -> Base[31:24]

    dq 0                    ; null
    ; descriptor código (base=0, limit=0xFFFFF, gran=4K)
    dw 0xFFFF               ; limit low
    dw 0x0000               ; base low
    db 0x00                 ; base mid
    db 10011010b            ; access: present, ring0, code, readable
    db 11001111b            ; gran=1, 32-bit, limit high=0xF
    db 0x00                 ; base high
    ; descriptor datos
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b            ; access: present, ring0, data, writable
    db 11001111b
    db 0x00
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1
    dd gdt_start


;Debo revisar eso de gdt porque sinceramente no lo entiendo, de donde sale tanto número? instrucciones? Sustos por lo menos

[bits 32]
pm_start:
    ; Cargar selectores de datos (0x10 = descriptor 2 en tu GDT)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000
    mov ebp, esp

    ; DEBUG: escribir '**' ANTES de llamar al kernel
    mov byte [0xB8000], '*'
    mov byte [0xB8001], 0x0F
    mov byte [0xB8002], '*'
    mov byte [0xB8003], 0x0F

    ; Llamar al kernel cargado en 0x00008000
    mov eax, 0x8000
    call eax

.hang:
    jmp .hang

[bits 16]

; --- Datos ---
msg db 'Virus Payal==> ',  0 
BootDrive db 0

KernelSects db 2             ; Depende del tamaño real (kernel.bin / 512 redondeado hacia arriba) 
; Se comprueba su tamaño con stat -f%z kernel.bin , y sus sectores con echo $((($(stat -f%z kernel.bin)+511)/512))
disk_msg db 'Error disco',0

progress_msg db 0x0D, 0x0A, '[', 0
done_msg db '] OK', 0x0D, 0x0A, 0



times 510 - ($ - $$) db 0    ; 8) Relleno hasta byte 510
dw 0xAA55                    ; 9) Firma 0xAA55 al final del sector