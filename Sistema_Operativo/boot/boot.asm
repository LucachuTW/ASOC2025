[org 0x7C00]                 ; Direccion de origen estándar

cli                          ; Limpia interrupciones para que no se corrompa la memoria, porque no conocemos los estados que nos da la BIOS
xor ax, ax                   ; Iguala ax a 0 porque al cumplirse ambos es una condición falsa
mov ds, ax                   ; DS = 0   -> datos en segmento base 0
mov es, ax                   ; ES = 0   -> útil para copiar/cargar a memoria
mov ss, ax                   ; SS = 0   -> segmento de pila
mov sp, 0x7C00               ; SP = 0x7C00 -> top de la pila (escalamos al origen tras reiniciar las banderas de interrupciones)
sti                          ; Reactiva interrupciones
cld                          ; Limpia DF (aloca y opcionalmente da 6 bits de valor a cada inicializador): las ops de cadena avanzan SI/DI hacia delante

mov [BootDrive], dl          ; Guarda el número de disco de arranque que te da el BIOS en DL, que suele ser el bit "a la izquierda" de dx, generalmente empleado como gestor de posición

mov si, msg                  ; 6) SI apunta a la cadena terminada en 0
call print_string            ;    imprime "OS:)" usando BIOS

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
    lodsb                    ; AL = [DS:SI], SI++
    test al, al              ; ¿fin de cadena?
    jz .done
    call print_char
    jmp .next
.done:
    ret

; --- Datos ---
msg db 'VirusPayal:)',  0 
BootDrive db 0

times 510 - ($ - $$) db 0    ; 8) Relleno hasta byte 510
dw 0xAA55                    ; 9) Firma 0xAA55 al final del sector