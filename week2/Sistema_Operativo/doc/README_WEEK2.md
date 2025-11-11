# Descripción y funcionamiento de week2/Sistema_Operativo

Este documento explica qué contiene la carpeta `week2/Sistema_Operativo`, cómo funciona el flujo de arranque que implementa y dónde buscar/editar las piezas principales. Está pensado para desarrolladores que quieran entender o extender el bootloader, el pseudokernel y el módulo post-boot.

## Estructura principal

- `boot/`
  - `stage1.asm` — MBR (512 bytes). Inicializa registros, muestra un menú mínimo de selección de disco, carga `stage2` en 0x7E00 y salta a él. Está cuidadosamente optimizado para caber en 512 bytes.
  - `stage2.asm` — Bootloader extendido (1024 bytes). Carga el pseudokernel desde disco (sectores 4+ -> 0x10000), escribe un header verificable en `0x7E00` para comunicar estado al kernel, habilita A20, configura la GDT y salta al pseudokernel en 0x10000.

- `kernel_boot/`
  - `start.S` — punto de entrada en modo protegido (32-bit). Configura la pila y llama a `kmain()`.
  - `kernel.c` — lógica principal del pseudokernel: UI, comprobaciones HW, sondeo del módulo opcional, validación y ejecución del módulo.
  - `kernelfunciones.c` — utilidades: output VGA, barras de progreso, lectura ATA PIO, parseo de cabeceras de módulo, manejo de teclado.
  - `kernel.h` — definiciones, direcciones importantes (p. ej. `MODULE_LOAD_ADDRESS`), prototipos y macros de color.

- `kernel_post/` (módulo post-boot) // aquí iría el verdadero kernel
  - `module_header.S` — cabecera del módulo (magic, versión, tamaño, offset de entry).
  - `module_entry.c` — ejemplo/demo de módulo que se carga en `MODULE_LOAD_ADDRESS` (0x0012000) y se ejecuta desde su offset de entry.
  - `modlib.c` / `modlib.h` — pequeñas utilidades para módulos (VGA, teclado, formatos) para mantener el módulo independiente del kernel.
  - `module.ld` — linker script para el módulo, vinculado para ejecutarse en `MODULE_LOAD_ADDRESS`.

- `doc/`
  - `funcionamiento.md` — descripción general (ya existente).
  - `README_WEEK2.md` — este archivo, con información práctica y pasos de uso.

- `Makefile` y `linker.ld` — scripts de construcción y layout del kernel (`linker.ld` vincula el kernel en 0x10000).

## Flujo de arranque (resumen)

1. BIOS ejecuta el MBR (`stage1.asm`) desde el sector 0.
2. `stage1` carga `stage2` en 0x7E00 (sectores 2-3) y salta a él (hay otra versión del programa donde tiene una especie de sistema de selección de discos, por desgracia no funciona bien del todo porque hay que hardcodear partes).
3. `stage2` (en modo real 16-bit) carga el kernel (sectores 4+ -> 0x10000). Cuando la carga es correcta, escribe un header verificable en 0x7E00 (word magic + versión + disco + kernel_sects) para que el kernel lo pueda comprobar. Luego habilita A20, carga GDT, cambia a modo protegido y salta a 0x10000.
4. El pseudokernel (32-bit) arranca en `_start`, inicializa subsistemas y ejecuta `kmain()`.
5. El kernel realiza chequeos HW, sonda la presencia de un módulo post-boot en el LBA `3 + kernel_sects`, lee su primer sector en `MODULE_LOAD_ADDRESS` para validar magic (`MOD0` o `MBIN`), muestra información y (si el usuario lo consiente) carga/ejecuta el módulo.

## Formato del header entre stage2 y kernel

- Location: `0x7E00` (start of stage2). Para evitar que el kernel acepte datos inválidos, `stage2` escribe un header verificable solo después de cargar el kernel.
- Layout:
  - offset 0..1: WORD MAGIC = 0x5453 ('S' 'T')
  - offset 2: byte VERSION (actualmente 1)
  - offset 3: byte DISK (valor DL/boot drive)
  - offset 4: byte KERNEL_SECTS (nº de sectores del kernel cargado)

El kernel debe verificar `MAGIC == 0x5453` antes de fiarse de los demás campos (y así hace `kernel.c`).

## Formato de módulo (kernel_post)

- Cabecera soportada: legacy `MOD0` o nuevo `MBIN` (ambos aceptados por `mod0_parse`). De todos modos simplemente busqué que tipo de cosas son las que suelen ser elevantes para mostrar y las puse xd. 
- Campos típicos en cabecera (ejemplo): magic (4), version (u16), length (u16/u32), entry_off (u32). El kernel valida la cabecera y el offset de entry antes de ejecutar.
- El módulo se carga (temporalmente) en `MODULE_LOAD_ADDRESS` (definido en `kernel.h` como `0x00012000`).

## Cómo funciona module_probe y qué es LBA

La detección y validación del módulo post-boot la hace la función `module_probe()` del pseudokernel. Aquí tienes una explicación detallada paso a paso y por qué se usa LBA para localizar el módulo en la imagen.

- Qué es LBA (Logical Block Addressing):
  - LBA es un esquema para identificar sectores en un disco por número lógico (0-based). En esta imagen raw se usa LBA para referirse al número de sector desde el inicio del fichero `os-image.bin`.
  - En el proyecto la convención es que el kernel empieza en LBA 3 (sector físico 4, 1-based). Por tanto el módulo, si existe, empieza en LBA = 3 + KS_COUNT (donde `KS_COUNT` es el número de sectores que ocupa el kernel y lo escribe `stage2` en el header).

- Flujo de `module_probe()` (resumido):
  1. Calcular LBA del módulo: lee `KS_COUNT` desde el header que dejó `stage2` y calcula `mod_lba = 3 + KS_COUNT`.
 2. Leer el primer sector (probe): realiza una lectura ATA/LBA del `mod_lba` y guarda el sector en un buffer temporal (`g_module_buf`).
 3. Comprobar magic/cabecera: analiza el buffer buscando los magics soportados (`MOD0`, `MBIN`, etc.). Si no hay magic válido, el probe falla y el kernel sigue sin módulo.
 4. Parsear campos esenciales: si el magic es válido, extrae versión, longitud (bytes o sectores) y `entry_off` (offset donde está la función de entrada dentro del módulo).
 5. Validaciones de seguridad:
    - La lectura tuvo éxito.
    - `entry_off` está dentro de los límites de la longitud declarada (no se ejecuta fuera del módulo).
    - La longitud solicitada no excede los límites razonables ni el tamaño de la imagen (esto evita lecturas fuera del fichero / truncado).
 6. Resultado del probe:
    - Si alguna validación falla, `module_probe()` devuelve fallo y el kernel informa "no módulo" o "cabecera inválida".
    - Si todo es válido, el kernel muestra la información del módulo (cabecera, dump parcial) y ofrece al usuario la opción de cargar/ejecutar.

- Carga y ejecución (si se acepta):
  - `module_finalize_after_bar()` o función equivalente lee los sectores adicionales del módulo (según la longitud leída) desde `mod_lba` a `MODULE_LOAD_ADDRESS` (0x0012000).
  - Una vez copiado el módulo a memoria, el kernel valida `entry_off` y llama a la función de entrada (por ejemplo saltando a `MODULE_LOAD_ADDRESS + entry_off`).
  - A la vuelta, el módulo debe haber preservado el estado que el kernel espera (ESP, buffers), y el kernel continúa.

- Ejemplos de casos:
  - Módulo ausente: primer sector no contiene magic → probe falla (salida: "no módulo detectado").
  - Módulo válido: probe detecta `MBIN` o `MOD0`, parsea longitud y entry_off válidos → se permite cargar y ejecutar.
  - Módulo corrupto: probe lee datos pero la cabecera está dañada o `entry_off` fuera de rango → probe falla y no se ejecuta nada.

- Riesgos y mitigaciones:
  - Si `KS_COUNT` es incorrecto, LBA calculado será erróneo → probe leerá basura. Mitigación: validar `KS_COUNT` con checksum en el header.
  - Si `IMAGE_SIZE` es demasiado pequeño y la imagen fue truncada, el módulo puede estar incompleto. Por eso el Makefile ahora amplía la imagen si el contenido concatenado excede `IMAGE_SIZE`.
  - Siempre validar `entry_off` y longitud antes de ejecutar para evitar saltos a memoria inválida.

Dónde mirar en el código:
- `kernel_boot/kernelfunciones.c` — implementación de `module_probe()`, `module_finalize_after_bar()` y `ata_read28()`.
- `kernel_boot/kernel.h` — macros como `MODULE_LOAD_ADDRESS` y offsets del header que escribe `stage2`.
- `boot/stage2.asm` — escribe el header con `KS_COUNT` (kernel sectors) que `module_probe()` usa para calcular LBA.
- `kernel_post/` — ejemplo de módulo (`module_header.S`, `module_entry.c`, `modlib.c`) para ver cómo se genera la cabecera esperada.

## Direcciones y convenciones importantes

- `stage2` load address: 0x7E00 (ejecución en modo real) — el header se escribe en 0x7E00.
- `kernel` link/load address: 0x10000 (definido en `linker.ld`).
- `MODULE_LOAD_ADDRESS`: 0x0012000 — donde se coloca y ejecuta el módulo post-boot.
- Zona compartida entre stage2 y kernel: `STAGE2_BASE = 0x7E00`. El pseudokernel lee el magic/versión/ks_count desde ahí, es donde debería estar el kernel real.

- De haber errores por solapamiento de código, tocaría mover las direcciones de memoria a otras donde no se lee algo indebido.

## Cómo construir y probar

Recomendación desde la raíz `week2/Sistema_Operativo`:

```
make clean
make
make run   # lanza qemu con os-image.bin
```

- `make` genera `stage1.bin` (512B), `stage2.bin` (1024B), compila y empaqueta el kernel (`kernel.bin`) y el módulo (`module_post.bin`) y concatena `os-image.bin`.
- `make run` lanza QEMU con `os-image.bin`. El make también hace el cálculo de la posición del kernel, un cambio que si no sería manual.

## Truncado y ampliación de la imagen

El `Makefile` define una variable `IMAGE_SIZE ?= 1440k` (por defecto 1.44MB) que controla el tamaño final de `os-image.bin` y de las imágenes de prueba (`test-stage1`, `test-stage2`). Puedes sobrescribirla en la línea de comandos, por ejemplo:

```
make IMAGE_SIZE=10M
```

Comportamiento concreto implementado en el `Makefile`:

- Paso 1: concatena los componentes (stage1, stage2, pseudokernel padded, módulo) en el archivo temporal final, p. ej. `os-image.bin`.
- Paso 2: calcula el tamaño real en bytes del archivo concatenado (`actual`).
- Paso 3: convierte el valor `IMAGE_SIZE` proporcionado (soporta sufijos `k`, `M`, `G`, `T` en mayúsculas o minúsculas) a bytes (`desired`). La conversión acepta enteros como `1440k`, `10M`, `1G`. Si pasas un número con decimales (por ejemplo `1.5M`) se truncará a entero durante la conversión.
- Paso 4: compara `actual` vs `desired`:
  - Si `actual > desired`, el `Makefile` AMPLÍA la imagen final al tamaño `actual` (usando `truncate -s actual`) para no perder bytes que contienen el kernel o el módulo. El make imprimirá una línea indicando que la imagen concatenada supera `IMAGE_SIZE` y que la ha ampliado a `actual` bytes.
  - Si `actual <= desired`, el `Makefile` trunca (o expande con ceros) la imagen al tamaño `desired` como antes (`truncate -s desired`).

Esto protege contra un caso habitual: concatenar un kernel grande o un módulo real y que luego `truncate` silenciosamente recorte bytes útiles si `IMAGE_SIZE` era demasiado pequeño. Con la lógica actual, el contenido concatenado no se pierde.

Notas y consideraciones:

- La conversión de sufijos se realiza con un pequeño fragmento `awk` en la receta del Makefile y es compatible con macOS/Linux. Para obtener el tamaño del archivo se intenta `stat -f%z` (BSD/macOS) y, si no está disponible, `stat -c%s` (GNU stat).
- Si tu entorno no tiene `stat` con esas opciones, la receta puede fallar; si pasa, dímelo y lo adapto a tu entorno concreto.
- Si prefieres política alternativa (por ejemplo: fallar y pedir explícitamente una `IMAGE_SIZE` mayor en vez de ampliar automáticamente), puedo cambiar la receta para que aborte en ese caso.

Ejemplo rápido para forzar la ampliación y ver el mensaje informativo:

```fish
make clean
make IMAGE_SIZE=512k
# Si la concatenación genera >512k, verás: "Imagen concatenada X bytes supera IMAGE_SIZE=512k (Y bytes). Ampliando imagen a X bytes."
```

## Depuración y puntos de control

- Si el kernel no muestra el header (mensaje "Stage2: no header detectado"), revisa que `stage2.asm` escribió correctamente el magic en 0x7E00 y que la llamada a INT 13h para leer el kernel fue exitosa.
- Para inspeccionar valores en tiempo de ejecución, puedes modificar `stage2.asm` para imprimir debug (ya hay mensajes tipo `[Stage2]`) o hacer que escriba un pattern reconocible en otra dirección.
- Si el módulo no se detecta:
  - Asegúrate de que el `module_post.bin` esté ubicado inmediatamente después del kernel en la imagen (Makefile empaqueta así).
  - `module_probe()` calcula LBA = 3 + kernel_sects; si kernel_sects es incorrecto, el LBA será erróneo.
  - Usa `print` en stage2 para verificar `KS_COUNT` y mensaje de OK tras la lectura.

## Potenciales Mejoras

- Añadir checksum: en `stage2` calcula un checksum del kernel y escríbelo en el header; el kernel lo valida antes de confiar.
- Soporte LBA ext: reemplazar las lecturas CHS de BIOS en stage2 por llamadas INT13h extend o implementar lectura ATA PIO en stage2 si necesitas más flexibilidad.
- Persistencia: se eliminó la escritura a CMOS por simplicidad; si necesitas persistencia entre reinicios, reserva y escribe un sector en disco (más portable).

---

## Diagramas: layout del disco y sectores

Este esquema muestra cómo `Makefile` empaqueta la imagen final `os-image.bin` y en qué sectores vive cada componente. Los números de sector son 1-based (sector 1 = primer sector físico):

```
DISCO (imagen raw) - sectores (ejemplo)
------------------------------------------------------------
Sector 1:   stage1 (512 bytes)                 <- MBR / stage1
Sector 2-3: stage2 (1024 bytes)                <- stage2 (2 sectores)
Sector 4..N: kernel (KS_COUNT sectores)       <- kernel.bin (padded a múltiplo de 512)
Sector N+1:  module_post.bin (512 bytes)      <- módulo post-boot (si existe)
Resto:      espacio libre (cálculo automático).
------------------------------------------------------------

LBA (para lectura por ATA) usado internamente:
- kernel LBA base = 3 (0-based) -> corresponde a sector físico 4 (1-based)
- módulo LBA base = 3 + KS_COUNT

```

Nota: el kernel se carga en 0x10000 y el módulo se coloca temporalmente en `MODULE_LOAD_ADDRESS` = 0x0012000.

## Ejemplos de salida del kernel (casos típicos)

Los siguientes ejemplos son salidas simuladas de la UI del kernel (`kmain`) para ayudar a entender cómo se ve cada caso. Las líneas importantes están marcadas.

1) Arranque normal, sin módulo disponible

```
               PseudoKernel
      Virus Payal OS Bootloader by Emefedez
=========================================
  Modulo opcional: 0x00012000 NO

  Modo protegido activado  [ OK ]
  Segmentos configurados   [ OK ]
  GDT cargada              [ OK ]
  A20 habilitada           [ OK ]
  Acceso VGA               [ OK ]

  PseudoKernel:  0x00010000 - 0x00011234 (4660 bytes)
  Stack (ESP):   0x00090000
  VGA:           0x000B8000

  Stage2: no header detectado, usando fallback

=========================================
      Sistema listo
=========================================
```

Explicación: `module_probe()` no encontró magic `MOD0/MBIN` en el LBA esperado. El kernel informa que no hay módulo y continúa.

2) Arranque con módulo válido (sondeado y ejecutable)

```
               PseudoKernel
      Virus Payal OS Bootloader by Emefedez
=========================================
  Modulo opcional: 0x00012000 SONDEADO [ OK ]
  [---====------] (barra de progreso)
  Copia modulo: OK
  -- Detalles modulo --
  Cabecera: OK ver=1 len=1024 entry_off=64 (valido)
  Dump[0..15]: 4D 42 49 4E 01 00 00 04 ...

  (Pulsa cualquier tecla para ejecutar el modulo)

  ESP antes: 0x0008F000
  ... (salida del modulo) ...
  ESP despues: 0x0008F000
  Retorno modulo

  [Modulo finalizado - 3s]

  PseudoKernel
  Modulo opcional:     0x00012000 CARGADO
  ... (resto de info) ...
```

Explicación: `module_probe()` detectó magic (`MBIN`/`MOD0`), `module_finalize_after_bar()` cargó sectores adicionales según la cabecera y el kernel puede ejecutar la función `entry` dentro del módulo.

3) Arranque con módulo presente pero corrupto/inválido

```
               PseudoKernel
      Virus Payal OS Bootloader by Emefedez
=========================================
  Modulo opcional: 0x00012000 SONDEADO
  Copia modulo: FALLO
  -- Detalles modulo --
  Cabecera: NO HEADER
  Dump[0..15]: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

  Entry invalido: no se ejecuta modulo

  [Modulo finalizado - 3s]

  PseudoKernel
  Modulo opcional:     0x00012000 (carga incompleta)
  ... (resto de info) ...
```

Explicación: el primer sector del módulo no contiene el magic esperado; el kernel marca fallo y no ejecuta el módulo.

## Ejemplo rápido de depuración (qué mirar cuando algo falla)

- Si el kernel muestra "Stage2: no header detectado": comprobar que `stage2` haya impreso `OK` tras leer el kernel y que `KS_COUNT` es correcto. Verifica `stage2.bin` y `kernel_sects.inc`.
- Si `module_probe()` devuelve falsos negativos: comprobar LBA calculado = `3 + KS_COUNT`. Puedes forzar `KS_COUNT` variable en `kernel_sects.inc` para pruebas.
- Si el módulo se detecta pero la ejecución falla: valida `entry_off` dentro de `[0, sector_size)` y que el módulo esté enlazado para `MODULE_LOAD_ADDRESS`.


