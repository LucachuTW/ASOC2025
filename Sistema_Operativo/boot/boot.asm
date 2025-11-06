[org 0x7C00]                 ; Direccion de origen estándar
bits 16                      ;cuestiones de compatibilidad

cli                          ; Limpia interrupciones para que no se corrompa la memoria, porque no conocemos los estados que nos da la BIOS
xor ax, ax                   ; Iguala ax a 0 porque al cumplirse ambos es una condición falsa; además, no podemos escribir directamente a ds/es; podría ponerse un 0 en vez del segundo ax
mov ds, ax                   ; DS = 0   -> datos en segmento base 0
mov es, ax                   ; ES = 0   -> útil para copiar/cargar a memoria
mov ss, ax                   ; SS = 0   -> segmento de pila
mov sp, 0x7B00               ; SP = 0x7C00 -> top de la pila (escalamos al origen tras reiniciar las banderas de interrupciones) !!!ESTO ES EL STACK!!! Se recomienda ponerlo anterior a la posición de inicio real para tener un mayor espacio
sti                          ; Reactiva interrupciones
cld                          ; Limpia DF (aloca y opcionalmente da 6 bits de valor a cada inicializador): las ops de cadena avanzan SI/DI hacia delante

mov [BootDrive], dl          ; Guarda el número de disco de arranque que te da el BIOS en DL, que suele ser el bit "a la izquierda" de dx, generalmente empleado como gestor de posición

mov si, msg                  ; 6) SI apunta a la cadena terminada en 0
call print_string            ;   emplea una instrucción de la BIOS para imprimir

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

; --- Datos ---
msg db 'Virus Payal==> ',  0 
BootDrive db 0

times 510 - ($ - $$) db 0    ; 8) Relleno hasta byte 510
dw 0xAA55                    ; 9) Firma 0xAA55 al final del sector