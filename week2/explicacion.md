# 📘 Documentación Completa del Proyecto: Sistema Operativo Básico

## 📋 Índice

1. Visión General
2. Arquitectura del Sistema
3. Bootloader (boot.asm)
4. Transición a Modo Protegido
5. Entry Point del Kernel (start.S)
6. Kernel en C (kernel.c)
7. Linker Script (linker.ld)
8. Sistema de Compilación (Makefile)
9. Flujo Completo de Ejecución
10. Decisiones de Diseño
11. Memoria y Direccionamiento
12. Debugging y Troubleshooting

---

## 🎯 Visión General

### ¿Qué es este proyecto?

Este proyecto implementa un **sistema operativo minimalista desde cero** que demuestra los conceptos fundamentales del arranque de un ordenador x86:

- **Bootloader personalizado** que carga código desde disco
- **Transición de modo real (16-bit) a modo protegido (32-bit)**
- **Kernel básico en C** con capacidades de salida por pantalla
- **Sistema de compilación automatizado** con Makefile

### Objetivos Educativos

1. Comprender el proceso de arranque de una PC x86
2. Aprender la transición entre modos de CPU
3. Integrar código assembly con C
4. Gestionar memoria a bajo nivel
5. Crear una imagen de disco arrancable

### Tecnologías Utilizadas

- **NASM** (Netwide Assembler) para código assembly
- **GCC cross-compiler** (x86_64-elf-gcc) para código C
- **GNU ld** para enlazado
- **QEMU** para emulación y pruebas
- **Make** para automatización

---

## 🏗️ Arquitectura del Sistema

### Estructura de Archivos

```
Sistema_Operativo/
├── boot/
│   └── boot.asm          # Bootloader (512 bytes, modo real 16-bit)
├── kernel/
│   ├── start.S           # Entry point del kernel (assembly 32-bit)
│   ├── kernel.c          # Lógica principal del kernel (C)
│   └── kernel.h          # Definiciones y prototipos
├── linker.ld             # Script del enlazador
├── Makefile              # Sistema de compilación
└── [generados en build]
    ├── boot.bin          # Bootloader compilado (512 bytes)
    ├── kernel.bin        # Kernel compilado (binario plano)
    └── os-image.bin      # Imagen final arrancable
```

### Mapa de Memoria

```
0x00000000 ┌─────────────────────────────────┐
           │ Tabla de Vectores de            │
0x000003FF │ Interrupción (IVT)              │
           ├─────────────────────────────────┤
0x00000400 │ BIOS Data Area (BDA)            │
0x000004FF ├─────────────────────────────────┤
           │                                 │
0x00000500 │ Área libre (usable)             │
           │                                 │
0x00007BFF ├─────────────────────────────────┤
0x00007C00 │ Boot Sector (bootloader)        │ ← La BIOS carga aquí
0x00007DFF ├─────────────────────────────────┤
           │ Área temporal/libre             │
0x00007FFF ├─────────────────────────────────┤
0x00008000 │ KERNEL                          │ ← Cargado por bootloader
           │ - start.S (entry point)         │
           │ - kernel.c (código C)           │
           │ - .rodata (strings)             │
           │ - .data (variables globales)    │
           │ - .bss (datos no inicializados) │
0x0008XXXX ├─────────────────────────────────┤
           │ Stack (crece hacia abajo)       │
0x00090000 │ ← ESP inicial                   │
           ├─────────────────────────────────┤
           │ Memoria extendida...            │
0x000A0000 ├─────────────────────────────────┤
0x000B8000 │ VGA Text Mode Buffer            │ ← 80x25 caracteres
0x000BFFFF ├─────────────────────────────────┤
           │ ROM BIOS, etc.                  │
0xFFFFFFFF └─────────────────────────────────┘
```

---

## 💾 Bootloader (boot.asm)

### Propósito

El bootloader es el **primer código que ejecuta la CPU** después del POST (Power-On Self-Test). Su misión:

1. Inicializar el entorno de modo real
2. Mostrar mensajes visuales (feedback al usuario)
3. Cargar el kernel desde disco a memoria
4. Preparar la transición a modo protegido
5. Transferir control al kernel

### Análisis Detallado del Código

#### 1. Directivas Iniciales

```nasm
[org 0x7C00]                 ; Dirección de origen estándar
[bits 16]                    ; Cuestiones de compatibilidad
```

**¿Por qué `0x7C00`?**
- Es una convención histórica de IBM PC
- La BIOS carga el boot sector (primeros 512 bytes del disco) en esta dirección
- `[org 0x7C00]` le dice a NASM que todas las etiquetas/direcciones se calculen relativas a esta base

**¿Por qué `[bits 16]`?**
- La CPU arranca en **modo real** (arquitectura 8086 compatible)
- Sólo puede direccionar 1 MB de RAM (20 bits)
- Usa segmentación: dirección física = (segmento × 16) + offset

#### 2. Inicialización Crítica

```nasm
cli                          ; Limpia interrupciones
xor ax, ax                   ; AX = 0
mov ds, ax                   ; DS = 0 → datos en segmento base 0
mov es, ax                   ; ES = 0 → útil para copiar/cargar a memoria
mov ss, ax                   ; SS = 0 → segmento de pila
mov sp, 0x7C00               ; SP = 0x7C00 → stack crece hacia abajo desde bootloader
sti                          ; Reactiva interrupciones
cld                          ; DF=0 → las instrucciones de cadena avanzan
```

**Decisión: ¿Por qué `cli` primero?**
- Durante el POST, el estado de la CPU es **indeterminado**
- Una interrupción antes de configurar `SS:SP` puede causar un **triple fault**
- `cli` garantiza que no ocurran interrupciones durante la inicialización

**Decisión: ¿Por qué `xor ax, ax` en vez de `mov ax, 0`?**
- `xor ax, ax` ocupa **2 bytes**: `31 C0`
- `mov ax, 0` ocupa **3 bytes**: `B8 00 00`
- En un boot sector limitado a 512 bytes, cada byte cuenta

**Decisión: ¿Por qué `cld` (Clear Direction Flag)?**
- La BIOS puede dejar `DF=1` (decrementar)
- `lodsb` (usado en `print_string`) lee bytes con auto-incremento de `SI`
- Si `DF=1`, `lodsb` **decrementa** `SI` → imprime caracteres al revés
- `cld` asegura que `DF=0` → `lodsb` incrementa `SI` correctamente

#### 3. Guardar Información de la BIOS

```nasm
mov [BootDrive], dl
```

**¿Por qué es crítico?**
- La BIOS pone el número del disco de arranque en `DL`:
  - `0x00` = Floppy A:
  - `0x80` = Primer disco duro
  - `0x81` = Segundo disco duro
- Necesitamos este valor para **cargar sectores adicionales** con `INT 0x13`
- Si no lo guardamos, perdemos esta información

#### 4. Interfaz de Usuario: Mensajes y Countdown

```nasm
mov si, msg
call print_string

; === COUNTDOWN 3-2-1 ===
mov si, countdown_prefix
call print_string

mov cx, 3
.countdown:
    push cx
    add cl, '0'
    mov al, cl
    call print_char
    
    call delay_1s
    
    ; Backspace para borrar el número
    mov al, 0x08
    call print_char
    mov al, ' '
    call print_char
    mov al, 0x08
    call print_char
    
    pop cx
    loop .countdown

mov si, go_msg
call print_string
```

**Decisión: ¿Por qué un countdown?**
- **Feedback visual**: confirma que el bootloader ejecuta
- **Debugging**: si se congela en el countdown, sabes que la carga/modo protegido falla
- **User experience**: da tiempo al usuario para ver el proceso

**Técnica: Conversión de número a ASCII**
```nasm
add cl, '0'   ; 3 → '3' (0x33), 2 → '2' (0x32), 1 → '1' (0x31)
```
- Aprovecha que los dígitos ASCII son secuenciales
- `'0'` = 0x30, `'1'` = 0x31, ..., `'9'` = 0x39

**Técnica: Backspace para actualizar en el mismo lugar**
```nasm
mov al, 0x08  ; Backspace → retroceder cursor
call print_char
mov al, ' '   ; Espacio → borrar carácter anterior
call print_char
mov al, 0x08  ; Backspace de nuevo → posicionar cursor
call print_char
```

#### 5. Funciones de I/O con BIOS

```nasm
print_char:
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    ret

print_string:
.next:
    lodsb                ; AL = [DS:SI], SI++
    test al, al          ; ¿AL == 0?
    jz .done
    call print_char
    jmp .next
.done:
    ret
```

**BIOS INT 0x10, AH=0x0E: Teletype Output**
- `AH=0x0E`: función "write character"
- `AL`: carácter a imprimir
- `BH`: página de video (0 = activa)
- `BL`: color (0x07 = gris claro sobre negro)

**Decisión: ¿Por qué `test al, al` en vez de `cmp al, 0`?**
- `test al, al` hace un AND lógico y actualiza flags
- Ocupa **2 bytes**: `84 C0`
- `cmp al, 0` ocupa **2 bytes**: `3C 00`
- Ambos funcionan, pero `test` es idiomático para comprobar si un valor es 0

**Técnica: `lodsb` (Load String Byte)**
- Lee `[DS:SI]` → `AL`
- Incrementa `SI` automáticamente (si `DF=0`)
- Equivalente a:
  ```nasm
  mov al, [si]
  inc si
  ```

#### 6. Delay (Espera Temporizada)

```nasm
delay_1s:
    mov ah, 0x86          ; BIOS: esperar microsegundos
    mov cx, 0x000F        ; CX:DX = 1,000,000 μs = 1 seg
    mov dx, 0x4240
    int 0x15
    ret
```

**BIOS INT 0x15, AH=0x86: Wait**
- `CX:DX`: tiempo en **microsegundos** (32 bits)
- 1,000,000 μs = 1 segundo
- `0x000F4240` = 1,000,000 en hexadecimal
  - `CX` = parte alta: `0x000F`
  - `DX` = parte baja: `0x4240`

**Limitación**: No todos los BIOS soportan esta función
- En hardware real antiguo puede no funcionar
- QEMU/VirtualBox sí lo soportan

#### 7. Habilitación de A20

```nasm
enable_a20:
    in al, 0x92
    or al, 00000010b
    out 0x92, al
    ret
```

**¿Qué es la línea A20?**
- En modo real, la CPU tiene **20 bits** de direccionamiento (1 MB)
- Pero la segmentación puede generar direcciones de 21 bits:
  ```
  Ejemplo: segmento=0xFFFF, offset=0x0010
  Dirección física = (0xFFFF × 16) + 0x0010 = 0x100000 (1 MB + 16 bytes)
  ```
- En el 8086 original, esto causaba **wrap-around** (volvía a 0x00000)
- Para compatibilidad, IBM PC deshabilitó el bit A20 por hardware
- En modo protegido necesitamos **todo el espacio de direcciones**

**Método " A20"**
- Puerto `0x64`, bit 1: habilita A20
- Es el método más universal (pero no universal)
- Alternativas: teclado (0x92), BIOS INT 0x15. 0x92 es más rápido

#### 8. Carga del Kernel desde Disco

```nasm
load_kernel:
    mov bx, 0x8000        ; ES:BX = 0x0000:0x8000 (destino)
    mov ah, 0x02          ; Función: leer sectores
    mov al, [KernelSects] ; Número de sectores a leer
    mov ch, 0x00          ; Cilindro 0
    mov cl, 0x02          ; Sector 2 (el boot es el 1)
    mov dh, 0x00          ; Cabeza 0
    mov dl, [BootDrive]   ; Disco guardado al inicio
    int 0x13
    jc disk_error
    ret

disk_error:
    mov si, disk_msg
    call print_string
    jmp $
```

**BIOS INT 0x13, AH=0x02: Read Sectors**
- `AH=0x02`: función "read sectors into memory"
- `AL`: número de sectores a leer (1-128)
- `CH`: cilindro (0-1023)
- `CL`: sector (1-63, **ojo: empieza en 1**)
- `DH`: cabeza (0-255)
- `DL`: drive number
- `ES:BX`: dirección de memoria destino
- **CF**: carry flag = 1 si error

**Decisión: ¿Por qué cargar en `0x8000`?**
- `0x7C00`-`0x7DFF`: ocupado por el bootloader
- `0x8000` es seguro, lejos de áreas críticas
- Coincide con la dirección base del linker script

**Decisión: ¿Por qué sector 2?**
- Sector 1 = boot sector (512 bytes)
- Sector 2+ = kernel
- La numeración de sectores **empieza en 1** (histórico)

#### 9. Global Descriptor Table (GDT)

```nasm
align 4
gdt_start:
    dq 0                        ; Descriptor nulo (obligatorio)
    ; Descriptor de código
    dw 0xFFFF                   ; Límite 0-15
    dw 0x0000                   ; Base 0-15
    db 0x00                     ; Base 16-23
    db 10011010b                ; Access: P=1, DPL=00, S=1, Type=1010 (code, read)
    db 11001111b                ; Flags: G=1, D=1, L=0, AVL=0, Límite 16-19=1111
    db 0x00                     ; Base 24-31
    ; Descriptor de datos
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b                ; Access: P=1, DPL=00, S=1, Type=0010 (data, write)
    db 11001111b
    db 0x00
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1  ; Tamaño de la GDT - 1
    dd gdt_start                ; Dirección base de la GDT
```

**¿Qué es la GDT?**
- **Global Descriptor Table**: tabla que define segmentos en modo protegido
- Cada entrada (descriptor) tiene **8 bytes**
- Define: base, límite, permisos, tipo

**Estructura de un descriptor (64 bits)**
```
Bits    | Descripción
--------|---------------------------------------------------------
0-15    | Límite 0:15 (tamaño del segmento)
16-31   | Base 0:15 (dirección inicial del segmento)
32-39   | Base 16:23
40      | A (Accessed): CPU lo pone a 1 al acceder
41      | R/W: Readable (código) o Writable (datos)
42      | DC: Direction/Conforming
43      | E: Executable (1=código, 0=datos)
44      | S: Descriptor type (1=code/data, 0=system)
45-46   | DPL: Descriptor Privilege Level (0=kernel, 3=user)
47      | P: Present (1=válido)
48-51   | Límite 16:19
52      | AVL: Available for system use
53      | L: Long mode (0 en modo protegido 32-bit)
54      | D/B: Default operand size (1=32-bit)
55      | G: Granularity (1=límite en páginas de 4KB)
56-63   | Base 24:31
```

**Nuestros descriptores**

*Código (selector 0x08)*:
- Base: `0x00000000`
- Límite: `0xFFFFF` (con G=1 → 4GB)
- Tipo: código ejecutable, readable
- DPL: 0 (ring 0 = kernel)

*Datos (selector 0x10)*:
- Base: `0x00000000`
- Límite: `0xFFFFF` (4GB)
- Tipo: datos, writable
- DPL: 0

**Decisión: Modelo de memoria plana (flat)**
- Base=0, Límite=4GB para todos los segmentos
- Simplifica la programación: direcciones lineales
- Segmentación es transparente

**¿Por qué el descriptor nulo?**
- Obligatorio en x86
- Selector 0 (null selector) provoca excepción si se usa
- Detecta errores: si cargas 0 en CS/DS/ES, CPU lanza #GP (General Protection Fault)

#### 10. Transición a Modo Protegido

```nasm
enter_protected_mode:
    cli
    lgdt [gdtr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp dword 0x08:pm_start
```

**Paso a paso**:

1. **`cli`**: deshabilitar interrupciones
   - En modo protegido, las interrupciones funcionan distinto (IDT)
   - Dejamos interrupciones deshabilitadas hasta configurar IDT

2. **`lgdt [gdtr]`**: cargar GDT
   - `lgdt` carga el registro GDTR (GDT Register)
   - GDTR contiene: límite (16 bits) + base (32 bits)

3. **`mov cr0, eax` con bit 0 = 1**: activar modo protegido
   - CR0 (Control Register 0) bit 0 = PE (Protection Enable)
   - PE=0: modo real
   - PE=1: modo protegido

4. **`jmp dword 0x08:pm_start`**: far jump
   - **Imprescindible**: flush del pipeline
   - `0x08`: selector del descriptor de código en GDT
   - `pm_start`: offset en 32 bits
   - Este salto actualiza `CS` con el nuevo selector

**¿Por qué `jmp dword`?**
- `jmp` normal es relativo (near jump)
- `jmp dword selector:offset` es absoluto (far jump)
- Far jump **recarga CS** con el selector especificado
- Sin far jump, CS seguiría con el valor de modo real → comportamiento indefinido

---

## 🔄 Transición a Modo Protegido

### Código 32-bit en el Bootloader

```nasm
[bits 32]
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    mov ebp, esp

    ; DEBUG
    mov byte [0xB8000], '*'
    mov byte [0xB8001], 0x0F
    mov byte [0xB8002], '*'
    mov byte [0xB8003], 0x0F

    mov eax, 0x8000
    call eax

.hang:
    hlt
    jmp .hang
```

**Análisis detallado**:

#### 1. Carga de Selectores de Segmento

```nasm
mov ax, 0x10      ; Selector del descriptor de datos
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
```

**¿Por qué `0x10`?**
- Estructura del selector (16 bits):
  ```
  Bits 15-3: Índice en la GDT/LDT (13 bits)
  Bit 2:     TI (Table Indicator): 0=GDT, 1=LDT
  Bits 1-0:  RPL (Requested Privilege Level)
  ```
- `0x10` = `0000 0000 0001 0000` binario
  - Índice: 2 (tercera entrada de la GDT, después de null y código)
  - TI: 0 (GDT)
  - RPL: 0 (ring 0)

**Decisión: Todos los segmentos usan el descriptor de datos**
- En modo protegido, los segmentos son solo selectores
- Con modelo plano (flat), todos apuntan a la misma región 0-4GB
- CS ya fue cargado en el far jump (`0x08`)

#### 2. Configuración del Stack

```nasm
mov esp, 0x90000
mov ebp, esp
```

**¿Por qué `0x90000` (576 KB)?**
- El stack crece **hacia abajo** (decrece)
- Desde `0x90000` hacia `0x8XXXX` hay ~512 KB de espacio
- El kernel empieza en `0x8000` y ocupa pocos KB
- Separación segura: kernel + stack no se sobreescriben

**¿Por qué `mov ebp, esp`?**
- `EBP` (Extended Base Pointer) marca el **frame inicial**
- Funciones en C usan `EBP` como referencia:
  ```c
  void func(int a, int b) {
      int local = a + b;
      // a = [ebp+8]
      // b = [ebp+12]
      // local = [ebp-4]
  }
  ```
- En `_start`, `EBP=ESP` indica "no hay frame anterior"

#### 3. Debug Visual

```nasm
mov byte [0xB8000], '*'
mov byte [0xB8001], 0x0F
mov byte [0xB8002], '*'
mov byte [0xB8003], 0x0F
```

**Memoria de Video VGA Text Mode**
- `0xB8000`-`0xBFFFF`: buffer de video (32 KB)
- Formato: 80×25 caracteres
- Cada carácter ocupa **2 bytes**:
  - Byte 0: código ASCII
  - Byte 1: atributo (color)

**Atributo `0x0F`**:
```
Bits 7-4: Background (0 = negro)
Bits 3-0: Foreground (15 = blanco brillante)
```

**Posicionamiento**:
- `0xB8000` = posición (0,0) → carácter
- `0xB8001` = posición (0,0) → atributo
- `0xB8002` = posición (1,0) → carácter
- `0xB8003` = posición (1,0) → atributo

**Resultado**: `**` en la esquina superior izquierda

**Decisión: ¿Por qué este debug?**
- **Triple fault**: si la transición falla, la CPU se resetea
- Si ves `**` en pantalla, **modo protegido funciona**
- Si no aparece, el error está en GDT o en el far jump

#### 4. Llamada al Kernel

```nasm
mov eax, 0x8000
call eax
```

**¿Por qué `call eax`?**
- `call` normal usa direcciones relativas (offset desde IP)
- `call eax` es un **call indirecto**: salta a la dirección en EAX
- Equivale a:
  ```nasm
  push eip + 5      ; Guardar dirección de retorno
  jmp [eax]         ; Saltar a 0x8000
  ```

**Decisión: ¿Por qué no `jmp 0x8000`?**
- `call` guarda la dirección de retorno en el stack
- Si el kernel termina (no debería), retornaría a `.hang`
- Más "limpio" que un `jmp` (aunque en práctica, el kernel no retorna)

#### 5. Loop Infinito

```nasm
.hang:
    hlt
    jmp .hang
```

**`hlt` (Halt)**
- Detiene la CPU hasta la próxima interrupción
- Ahorra energía (importante en laptops)
- Interrupciones están deshabilitadas (`cli`), así que CPU duerme indefinidamente

**¿Por qué `jmp .hang` después de `hlt`?**
- `hlt` puede ser interrumpido por NMI (Non-Maskable Interrupt)
- Si ocurre NMI, CPU continúa ejecutando
- `jmp .hang` vuelve a `hlt`

---

## 🚀 Entry Point del Kernel (start.S)

### Propósito

`start.S` es el **puente entre assembly y C**. Su único trabajo: crear un entorno válido para código C.

### Análisis del Código

```asm
.section .text
.global _start
.extern kmain

_start:
    cli                   # 1. Deshabilitar interrupciones
    mov $0x90000, %esp    # 2. Configurar stack pointer
    mov %esp, %ebp        # 3. Base pointer = stack pointer
    
    # 4. Debug: escribir '+' en VGA (posición 2)
    movb $'+', 0xB8004
    movb $0x0F, 0xB8005
    
    call kmain            # 5. Llamar a kmain() en C
hang:
    hlt
    jmp hang              # 6. Si kmain retorna, colgar
```

**Sintaxis AT&T vs Intel**
- Este código usa **sintaxis AT&T** (default de GCC/GNU as)
- Diferencias clave:
  ```
  Intel:  mov eax, 5       ; destino, fuente
  AT&T:   movl $5, %eax    # fuente, destino
  
  Intel:  mov eax, [ebx]
  AT&T:   movl (%ebx), %eax
  
  Intel:  movb [0xB8000], 'A'
  AT&T:   movb $'A', 0xB8000
  ```

#### 1. `cli` (Clear Interrupts)

```asm
cli
```

**¿Por qué de nuevo?**
- En `pm_start` (boot.asm) ya hicimos `cli`
- Pero `start.S` es independiente del bootloader
- Si cambiamos el bootloader, `start.S` sigue funcionando
- **Principio de defensa**: nunca asumir estado previo

#### 2. Configuración del Stack

```asm
mov $0x90000, %esp
mov %esp, %ebp
```

**Redundancia con `pm_start`**:
- Sí, `pm_start` ya configuró `ESP=0x90000`
- Pero nuevamente: **independencia**
- `start.S` debe funcionar aunque lo llamen desde otro bootloader

**Decisión: ¿Por qué no usar el stack que ya existe?**
- Podríamos ahorrar estas instrucciones
- Pero en sistemas reales, el bootloader puede usar su propio stack
- Reconfigurar asegura que el kernel tiene un stack limpio

#### 3. Debug Visual

```asm
movb $'+', 0xB8004
movb $0x0F, 0xB8005
```

**Posición en pantalla**:
- `0xB8000` y `0xB8001`: `*` (primer asterisco de `pm_start`)
- `0xB8002` y `0xB8003`: `*` (segundo asterisco)
- `0xB8004` y `0xB8005`: `+` (este código)

**Resultado visual**: `**+` en la esquina superior izquierda

**Decisión de debug progresivo**:
- Bootloader (modo real): imprime mensajes
- `pm_start` (32-bit): imprime `**`
- `start.S`: imprime `+`
- `kmain`: limpia pantalla e imprime banner

**Ventaja**: si el sistema se cuelga, sabes en qué fase ocurrió

#### 4. Llamada a `kmain`

```asm
call kmain
```

**Convención de llamada (cdecl)**
- Parámetros se pasan en el stack (de derecha a izquierda)
- Valor de retorno en `EAX`
- Caller limpia el stack
- `EAX`, `ECX`, `EDX` son "caller-saved" (pueden modificarse)
- `EBX`, `ESI`, `EDI`, `EBP` son "callee-saved" (deben preservarse)

**`kmain` no tiene parámetros**:
```c
void kmain(void);
```
- No hay que preparar nada en el stack
- `call kmain` simplemente:
  1. `push eip+5` (dirección de retorno)
  2. `jmp kmain`

#### 5. Loop de Seguridad

```asm
hang:
    hlt
    jmp hang
```

**¿Cuándo se ejecuta?**
- `kmain` tiene un loop infinito al final: `for(;;);`
- **No debería retornar nunca**
- Si retorna (bug en `kmain`), llegamos a `hang`

**Alternativa sin `hlt`**:
```asm
hang:
    jmp hang
```
- Funciona, pero consume CPU al 100%
- `hlt` es mejor práctica

---

## 🖥️ Kernel en C (kernel.c)

### Estructura General

```c
#include "kernel.h"

static uint16_t* vga = (uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

void clear_screen(void) { ... }
void putchar(char c, uint8_t color) { ... }
void print_string(const char* str, uint8_t color) { ... }
void print_hex(uint32_t value) { ... }
void kmain(void) { ... }
```

### Variables Globales

```c
static uint16_t* vga = (uint16_t*)VGA_ADDRESS;
```

**Decisión: `uint16_t*` en vez de `uint8_t*`**
- Cada celda de VGA son **2 bytes** (carácter + atributo)
- Usar `uint16_t*` permite escribir ambos de una vez:
  ```c
  vga[0] = 0x0F41;  // 'A' con atributo 0x0F
  ```
- Alternativa con `uint8_t*`:
  ```c
  uint8_t* vga = (uint8_t*)0xB8000;
  vga[0] = 'A';     // Carácter
  vga[1] = 0x0F;    // Atributo
  ```

**Decisión: `static` para ocultar símbolos**
- `static` hace que las variables sean locales a este archivo
- No se exportan al enlazador (no contaminan el namespace global)
- Mejor encapsulación

### Función: `clear_screen`

```c
void clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (COLOR_BLACK << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}
```

**Análisis**:
- `VGA_WIDTH * VGA_HEIGHT` = 80 × 25 = 2000 celdas
- Cada celda: `(atributo << 8) | carácter`
  - `COLOR_BLACK` = 0x00 → background negro, foreground negro
  - `' '` = 0x20 (espacio)
  - Resultado: `0x0020`

**Decisión: Llenar con espacios en vez de NULL**
- `0x0000` mostraría caracteres basura (NULL en muchas fuentes)
- `' '` (espacio) se ve como una celda vacía

### Función: `putchar`

```c
void putchar(char c, uint8_t color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int offset = cursor_y * VGA_WIDTH + cursor_x;
        vga[offset] = (color << 8) | c;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    
    // Scroll simple
    if (cursor_y >= VGA_HEIGHT) {
        cursor_y = VGA_HEIGHT - 1;
        // Mover líneas hacia arriba
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            vga[i] = vga[i + VGA_WIDTH];
        }
        // Limpiar última línea
        for (int i = 0; i < VGA_WIDTH; i++) {
            vga[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = (COLOR_BLACK << 8) | ' ';
        }
    }
}
```

**Gestión del Cursor**:
- `cursor_x`, `cursor_y`: posición actual (0-based)
- `offset = y × 80 + x`: conversión 2D → 1D

**Manejo de `\n` (newline)**:
- Resetea columna a 0
- Incrementa fila
- **No** escribe el carácter en pantalla (es un control)

**Word Wrap (ajuste de línea)**:
```c
if (cursor_x >= VGA_WIDTH) {
    cursor_x = 0;
    cursor_y++;
}
```
- Si llegamos al borde derecho, pasamos a la siguiente línea automáticamente

**Scroll (desplazamiento)**:
```c
if (cursor_y >= VGA_HEIGHT) {
    // Copiar líneas 1-24 a posiciones 0-23
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        vga[i] = vga[i + VGA_WIDTH];
    }
    // Borrar línea 24
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = (COLOR_BLACK << 8) | ' ';
    }
    cursor_y = VGA_HEIGHT - 1;
}
```

**Decisión: Scroll simple sin doble buffer**
- Scroll básico: mover memoria
- En sistemas reales, se usa **paginación** (cambiar base de VGA)
- O doble buffer (renderizar off-screen, luego swap)

### Función: `print_string`

```c
void print_string(const char* str, uint8_t color) {
    while (*str) {
        putchar(*str++, color);
    }
}
```

**Modismo C**: `*str++`
1. Leer `*str` (valor actual)
2. Incrementar `str` (apuntar al siguiente carácter)
3. Orden de operación: `*(str++)`

**Equivalente verbose**:
```c
char c = *str;
putchar(c, color);
str++;
```

### Función: `print_hex`

```c
void print_hex(uint32_t value) {
    char hex[] = "0x00000000";
    for (int i = 9; i >= 2; i--) {
        uint8_t nibble = value & 0xF;
        hex[i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
        value >>= 4;
    }
    print_string(hex, COLOR_WHITE);
}
```

**Conversión de entero a hexadecimal**:
- Un nibble (4 bits) = un dígito hexadecimal
- `value & 0xF`: extraer los 4 bits menos significativos
- `value >>= 4`: descartar esos 4 bits

**Mapeo nibble → carácter**:
```
0-9:  '0' + nibble       → '0'..'9'
10-15: 'A' + (nibble-10) → 'A'..'F'
```

**Orden de procesamiento**: de derecha a izquierda
- Primer nibble extraído → último carácter (`hex[9]`)
- Último nibble extraído → primer carácter después de "0x" (`hex[2]`)

**Ejemplo**: `value = 0x12AB`
```
Iteración 1: nibble = B (11) → hex[9] = 'B'
Iteración 2: nibble = A (10) → hex[8] = 'A'
Iteración 3: nibble = 2 (2)  → hex[7] = '2'
Iteración 4: nibble = 1 (1)  → hex[6] = '1'
Iteraciones 5-8: nibble = 0  → hex[5..2] = '0'
Resultado: "0x000012AB"
```

### Función Principal: `kmain`

```c
void kmain(void) {
    clear_screen();
    
    print_string("[ Virus Payal OS ]\n", COLOR_GREEN);
    print_string("================\n\n", COLOR_WHITE);
    
    print_string("Kernel cargado en: ", COLOR_WHITE);
    print_hex(0x8000);
    print_string("\n", COLOR_WHITE);
    
    print_string("Estado: ", COLOR_WHITE);
    print_string("OK\n", COLOR_GREEN);
    
    print_string("\nSistema iniciado correctamente.\n", COLOR_WHITE);
    
    // Bucle infinito
    for(;;);
}
```

**Decisión: `for(;;)` en vez de `while(1)`**
- Ambos son bucles infinitos
- `for(;;)` es idiomático en kernels (más "bare metal")
- GCC genera el mismo código assembly

**Orden de ejecución**:
1. Limpiar pantalla (borrar `**+` de debug)
2. Banner colorizado
3. Información del sistema
4. Confirmar estado OK
5. Colgar en bucle infinito

**Decisión: No usar `return`**
- `kmain` debería ser `void` y nunca retornar
- Si retorna, `start.S` lo captura en `hang`
- En kernels reales, retornar del punto de entrada es un bug fatal

---

## 🔗 Linker Script (linker.ld)

### Código Completo

```ld
ENTRY(_start)              # Punto de entrada: primera función a ejecutar
SECTIONS {
  . = 0x8000;              # Dirección base: el kernel comienza en 0x8000
  .text : { *(.text*) }    # Sección de código ejecutable
  .rodata : { *(.rodata*) } # Datos de solo lectura (strings constantes)
  .data : { *(.data*) }    # Datos inicializados (variables globales con valor)
  .bss : { *(.bss*) *(COMMON) } # Datos no inicializados (variables sin valor inicial)
}
```

### Análisis Detallado

#### `ENTRY(_start)`

**¿Qué hace?**
- Define el **símbolo de entrada** del ejecutable
- El linker marca `_start` como el punto de inicio
- Herramientas como `objdump` y debuggers usan esta información

**¿Por qué `_start` y no `main`?**
- `main` es convención de C (requiere runtime de libc)
- En bare metal, no hay runtime
- `_start` es la convención para entry points de bajo nivel

#### `. = 0x8000`

**Sintaxis**:
- `.` (punto) = **location counter** (contador de dirección)
- `. = 0x8000` establece el contador en `0x8000`
- Todas las secciones siguientes se ubicarán a partir de esta dirección

**Decisión: ¿Por qué `0x8000`?**
- Coincide con la dirección de carga en `boot.asm` (`mov bx, 0x8000`)
- Debe ser coherente:
  - Bootloader carga kernel en `0x8000`
  - Linker genera código asumiendo base `0x8000`
  - CPU salta a `0x8000` esperando encontrar `_start`

**¿Qué pasaría con otra dirección?**
```ld
. = 0x10000;  # Kernel en 64 KB
```
- Bootloader sigue cargando en `0x8000`
- Kernel espera estar en `0x10000`
- Referencias absolutas (como `vga = 0xB8000`) funcionan
- Referencias relativas (como `call func`) **fallan** (calculan mal offsets)

#### Sección `.text`

```ld
.text : { *(.text*) }
```

**Sintaxis**:
- `.text :` define una sección llamada `.text` en el output
- `{ *(.text*) }` incluye todas las secciones `.text*` de todos los archivos `.o`
- `*` es wildcard (comodín)

**Contenido**:
- Código ejecutable de `start.S`: `_start`, `hang`
- Código ejecutable de `kernel.c`: `kmain`, `clear_screen`, `putchar`, etc.

**Orden de archivos importa**:
```makefile
$(LD) -o kernel.elf start.o kernel.o
```
- `start.o` va primero → `_start` está al inicio de `.text`
- `kernel.o` va después → `kmain` sigue a `_start`

**Resultado en memoria**:
```
0x8000: _start (start.S)
0x800X: hang (start.S)
0x80XX: kmain (kernel.c)
0x80XX: clear_screen (kernel.c)
...
```

#### Sección `.rodata`

```ld
.rodata : { *(.rodata*) }
```

**Read-Only Data**:
- Strings literales: `"[ Virus Payal OS ]"`
- Constantes: `const int MAX = 100;`
- Tablas de datos

**Ejemplo en C**:
```c
const char* msg = "Hello";  // "Hello" va a .rodata
                             // msg (puntero) va a .data
```

**Ventaja de separar `.rodata`**:
- En sistemas con MMU (Memory Management Unit), `.rodata` se marca como read-only
- Intentar escribir en `.rodata` → **page fault**
- Detecta bugs: modificar strings literales es undefined behavior

#### Sección `.data`

```ld
.data : { *(.data*) }
```

**Datos inicializados**:
```c
int counter = 0;           // .data
char buffer[100] = {0};    // .data
```

**En el binario**:
- `.data` ocupa espacio en `kernel.bin`
- Los valores iniciales se almacenan
- Al cargar, se copian a RAM

#### Sección `.bss`

```ld
.bss : { *(.bss*) *(COMMON) }
```

**Block Started by Symbol (BSS)**:
- Variables sin inicializar o con valor 0
```c
int counter;               // .bss
char buffer[1024];         // .bss
static int x = 0;          // .bss (optimizado)
```

**Optimización**:
- `.bss` **no ocupa espacio en el binario**
- Solo guarda tamaño
- El loader (en nuestro caso, no hay) debe llenar con ceros

**¿Qué es `COMMON`?**
- En C, variables globales no inicializadas son "tentative definitions"
- El linker las pone en `COMMON` (shared symbols)
- `*(COMMON)` las fusiona en `.bss`

**Ejemplo**:
```c
// file1.c
int shared_var;

// file2.c
int shared_var;

// Sin COMMON → linker error (multiple definition)
// Con COMMON → se fusionan en una sola variable
```

### Orden de Secciones

**¿Por qué `.text` primero?**
- El entry point (`_start`) está en `.text`
- CPU salta a `0x8000` esperando código
- Si `.data` fuera primero, CPU ejecutaría **datos como código** → crash

**Layout en memoria**:
```
0x8000  ┌─────────────┐
        │   .text     │  Código ejecutable
0x8XXX  ├─────────────┤
        │  .rodata    │  Strings, constantes
0x8XXX  ├─────────────┤
        │   .data     │  Variables inicializadas
0x8XXX  ├─────────────┤
        │   .bss      │  Variables no inicializadas
0x8XXX  └─────────────┘
```

---

## ⚙️ Sistema de Compilación (Makefile)

### Variables de Configuración

```makefile
CC = x86_64-elf-gcc           # Compilador cross-compiler para i386
LD = x86_64-elf-ld            # Enlazador
OBJCOPY = x86_64-elf-objcopy  # Extractor de binarios
```

**¿Por qué cross-compiler?**
- Tu Mac usa compiladores para arquitectura ARM64 o x86-64
- El kernel necesita compilarse para **i386 (32-bit x86)**
- Cross-compiler: compilador que genera código para otra arquitectura

**Instalación**:
```bash
brew install x86_64-elf-gcc
```

**Alternativa**: usar `gcc -m32` (si tu sistema es x86-64)

### Target: `boot.bin`

```makefile
boot.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o boot.bin
```

**Opciones de NASM**:
- `-f bin`: formato binario plano (no ELF, no headers)
- Output directo: código máquina

**Resultado**:
- `boot.bin` tiene **exactamente 512 bytes**
- Byte 510-511: `0x55 0xAA` (firma de boot sector)

### Target: `start.o`

```makefile
start.o: kernel/start.S
	$(CC) -m32 -c kernel/start.S -o start.o
```

**Opciones**:
- `-m32`: generar código 32-bit (i386)
- `-c`: compilar sin enlazar (genera `.o`)

**Resultado**:
- `start.o` en formato ELF (Executable and Linkable Format)
- Contiene símbolos: `_start`, `hang`
- Relocations pendientes (direcciones sin resolver)

### Target: `kernel.o`

```makefile
kernel.o: kernel/kernel.c kernel/kernel.h
	$(CC) -m32 -ffreestanding -fno-pic -fomit-frame-pointer -nostdlib -Ikernel -c kernel/kernel.c -o kernel.o
```

**Opciones críticas**:

#### `-m32`
- Generar código 32-bit

#### `-ffreestanding`
- **Freestanding environment**: sin sistema operativo
- Desactiva optimizaciones que asumen libc
- Ejemplo: `memcpy` no se transforma en `__builtin_memcpy` (que requiere libc)

#### `-fno-pic`
- **PIC** = Position Independent Code
- PIC: código que funciona en cualquier dirección de memoria
- `-fno-pic`: código con direcciones absolutas
- En nuestro kernel, sabemos que estamos en `0x8000` → no necesitamos PIC
- PIC genera código más lento (usa GOT, Global Offset Table)

#### `-fomit-frame-pointer`
- Normalmente, GCC usa `EBP` como frame pointer:
  ```asm
  push ebp
  mov ebp, esp
  sub esp, 16       ; Reservar espacio para locales
  ...
  mov esp, ebp
  pop ebp
  ret
  ```
- Con `-fomit-frame-pointer`, GCC usa `EBP` como registro de propósito general
- Ventaja: un registro más disponible
- Desventaja: debugging más difícil (backtraces menos precisos)

#### `-nostdlib`
- No enlazar con libc (biblioteca estándar de C)
- No hay `printf`, `malloc`, `strlen`, etc.
- Debemos implementar todo manualmente

#### `-Ikernel`
- Añadir `kernel/` al path de include
- Permite `#include "kernel.h"` en vez de `#include "../kernel/kernel.h"`

### Target: `kernel.bin`

```makefile
kernel.bin: start.o kernel.o linker.ld
	$(LD) -m elf_i386 -T linker.ld -nostdlib -o kernel.elf start.o kernel.o
	$(OBJCOPY) -O binary kernel.elf kernel.bin
	@echo "Kernel:" $$(stat -f%z kernel.bin) "bytes," $$(((($$(stat -f%z kernel.bin)+511)/512))) "sectores"
```

**Paso 1: Enlazar**
```makefile
$(LD) -m elf_i386 -T linker.ld -nostdlib -o kernel.elf start.o kernel.o
```

**Opciones**:
- `-m elf_i386`: formato de salida ELF de 32 bits
- `-T linker.ld`: usar script de linker personalizado
- `-nostdlib`: no enlazar con libc
- **Orden**: `start.o kernel.o` (¡crítico!)

**Resultado**: `kernel.elf`
- Ejecutable ELF completo
- Headers ELF: tipo, arquitectura, entry point, secciones
- Secciones: `.text`, `.rodata`, `.data`, `.bss`
- Tabla de símbolos: `_start`, `kmain`, etc.

**Paso 2: Extraer binario**
```makefile
$(OBJCOPY) -O binary kernel.elf kernel.bin
```

**`objcopy -O binary`**:
- Elimina headers ELF
- Extrae solo el contenido de las secciones loadables
- Output: código máquina puro

**¿Por qué este paso?**
- El bootloader no entiende ELF
- Carga bytes directamente desde disco a `0x8000`
- Necesita código máquina plano sin metadatos

**Paso 3: Calcular sectores**
```makefile
@echo "Kernel:" $$(stat -f%z kernel.bin) "bytes," $$(((($$(stat -f%z kernel.bin)+511)/512))) "sectores"
```

**Fórmula**: `sectores = ⌈bytes / 512⌉`
- Redondeo hacia arriba: `(bytes + 511) / 512`
- Ejemplo: 
  - 400 bytes → (400+511)/512 = 911/512 = 1 sector
  - 600 bytes → (600+511)/512 = 1111/512 = 2 sectores

**Propósito**: informar al usuario
- Debes ajustar `KernelSects` en `boot.asm` manualmente
- Si el kernel crece, cambiar `KernelSects db 2` → `db 3`

### Target: `os-image.bin`

```makefile
os-image.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.bin
	dd if=/dev/zero bs=512 count=10 >> os-image.bin 2>/dev/null
```

**Concatenación**:
```bash
cat boot.bin kernel.bin > os-image.bin
```
- Byte 0-511: boot sector
- Byte 512+: kernel

**Padding con `dd`**:
```bash
dd if=/dev/zero bs=512 count=10 >> os-image.bin
```
- Añade 10 sectores (5 KB) de ceros
- Simula un disco más grande
- No es estrictamente necesario, pero algunos emuladores lo prefieren

**Resultado**: `os-image.bin`
- Imagen de disco arrancable
- Compatible con QEMU, VirtualBox, etc.

### Target: `run`

```makefile
run: os-image.bin
	qemu-system-x86_64 -drive format=raw,file=os-image.bin
```

**QEMU**:
- Emulador de CPU x86
- `-drive format=raw,file=os-image.bin`: disco raw (sin particiones)
- QEMU ejecuta BIOS virtual → carga boot sector → ejecuta kernel

### Target: `clean`

```makefile
clean:
	rm -f *.bin *.o *.elf
```

**Limpieza**:
- Elimina archivos generados
- Permite rebuild limpio: `make clean && make`

---

## 🔄 Flujo Completo de Ejecución

### Diagrama de Secuencia

```
┌──────┐
│ BIOS │ Power-On Self-Test (POST)
└───┬──┘
    │ 1. Inicializar hardware
    │ 2. Buscar dispositivo booteable
    │ 3. Leer sector 0 (boot sector) → 0x7C00
    │ 4. Verificar firma 0xAA55
    │ 5. JMP 0x0000:0x7C00
    ▼
┌────────────┐
│ boot.asm   │ [Modo Real 16-bit]
│ @ 0x7C00   │
└─────┬──────┘
      │ 1. CLI, inicializar segmentos
      │ 2. Mostrar "Virus Payal==>"
      │ 3. Countdown 3-2-1-GO
      │ 4. Barra de progreso [**********]
      │ 5. Cargar kernel (INT 0x13) @ 0x8000
      │ 6. Habilitar A20
      │ 7. Cargar GDT
      │ 8. CR0 |= 1 (activar modo protegido)
      │ 9. FAR JMP 0x08:pm_start
      ▼
┌────────────┐
│ pm_start   │ [Modo Protegido 32-bit]
│ @ 0x7CXX   │
└─────┬──────┘
      │ 1. Cargar selectores de segmento (0x10)
      │ 2. Configurar stack (ESP = 0x90000)
      │ 3. Escribir "**" en VGA @ 0xB8000
      │ 4. CALL 0x8000
      ▼
┌────────────┐
│ start.S    │ [Modo Protegido 32-bit]
│ @ 0x8000   │
└─────┬──────┘
      │ 1. CLI (seguridad)
      │ 2. Configurar ESP/EBP (0x90000)
      │ 3. Escribir "+" en VGA @ 0xB8004
      │ 4. CALL kmain
      ▼
┌────────────┐
│ kmain()    │ [C, Modo Protegido 32-bit]
│ @ 0x80XX   │
└─────┬──────┘
      │ 1. clear_screen()
      │ 2. print_string("[ Virus Payal OS ]")
      │ 3. print_hex(0x8000)
      │ 4. print_string("OK")
      │ 5. for(;;); // loop infinito
      ▼
    (Sistema en ejecución)
```

