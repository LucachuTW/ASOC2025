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
  - `module_entry.c` — ejemplo/demo de módulo que se enlaza como ELF y se ejecuta desde la `e_entry` del binario.
  - `modlib.c` / `modlib.h` — pequeñas utilidades para módulos (VGA, teclado, formatos) para mantener el módulo independiente del kernel.
  - `module.ld` — linker script ELF para el módulo, vinculado para ejecutarse en `MODULE_LOAD_ADDRESS`.
  - `module_header.S` — versión legacy de cabecera plana (se conserva a modo histórico, ya no se usa en la build actual).

- `doc/`
  - `funcionamiento.md` — descripción general (ya existente).
  - `README_WEEK2.md` — este archivo, con información práctica y pasos de uso.

- `Makefile` y `linker.ld` — scripts de construcción y layout del kernel (`linker.ld` vincula el kernel en 0x10000).

## Flujo de arranque (resumen)

1. BIOS ejecuta el MBR (`stage1.asm`) desde el sector 0.
2. `stage1` carga `stage2` en 0x7E00 (sectores 2-3) y salta a él (hay otra versión del programa donde tiene una especie de sistema de selección de discos, por desgracia no funciona bien del todo porque hay que hardcodear partes).
3. `stage2` (en modo real 16-bit) carga el kernel (sectores 4+ -> 0x10000). Cuando la carga es correcta, escribe un header verificable en 0x7E00 (word magic + versión + disco + kernel_sects) para que el kernel lo pueda comprobar. Luego habilita A20, carga GDT, cambia a modo protegido y salta a 0x10000.
4. El pseudokernel (32-bit) arranca en `_start`, inicializa subsistemas y ejecuta `kmain()`.
5. El kernel realiza chequeos HW, sonda la presencia de un módulo post-boot en el LBA `3 + kernel_sects`, valida que el primer sector contenga una cabecera ELF32 (`0x7FELF`) y, si el usuario lo consiente, lee todos los sectores del módulo, carga sus segmentos `PT_LOAD` en memoria física y salta a `e_entry`.

## Formato del header entre stage2 y kernel

- Location: `0x7E00` (start of stage2). Para evitar que el kernel acepte datos inválidos, `stage2` escribe un header verificable solo después de cargar el kernel.
- Layout:
  - offset 0..1: WORD MAGIC = 0x5453 ('S' 'T')
  - offset 2: byte VERSION (actualmente 1)
  - offset 3: byte DISK (valor DL/boot drive)
  - offset 4: byte KERNEL_SECTS (nº de sectores del kernel cargado)

El kernel debe verificar `MAGIC == 0x5453` antes de fiarse de los demás campos (y así hace `kernel.c`).

## Formato de módulo (kernel_post)

- El módulo se construye ahora como ELF32 (little endian) enlazado para x86 (`EM_386`).
- `module.ld` fija tanto `p_paddr` como `p_vaddr` de los segmentos en torno a `MODULE_LOAD_ADDRESS` para que el pseudokernel pueda copiar directamente con memoria física identidad.
- El kernel valida la cabecera ELF (`0x7F 'E' 'L' 'F'`), comprueba que `e_phentsize` y `e_phnum` sean coherentes y que la tabla de program headers quede dentro del tamaño real del archivo (`MODULE_SIZE_BYTES`).
- Durante la carga se recorren los headers `PT_LOAD`: cada uno se copia a la dirección física indicada (se respeta `p_filesz` y se rellena con ceros la zona `p_memsz - p_filesz`).
- La entrada ejecutable es `e_entry`. Se exige que `e_entry` caiga dentro de uno de los segmentos `PT_LOAD` antes de permitir la ejecución.
- `module_header.S` y el formato `MOD0/MBIN` permanecen en el repo solo como referencia histórica; la build actual ya no los utiliza.

## Cómo funciona module_probe y qué es LBA

La detección y validación del módulo post-boot la hace la función `module_probe()` del pseudokernel. Aquí tienes una explicación detallada paso a paso y por qué se usa LBA para localizar el módulo en la imagen.

- Qué es LBA (Logical Block Addressing):
  - LBA es un esquema para identificar sectores en un disco por número lógico (0-based). En esta imagen raw se usa LBA para referirse al número de sector desde el inicio del fichero `os-image.bin`.
  - En el proyecto la convención es que el kernel empieza en LBA 3 (sector físico 4, 1-based). Por tanto el módulo, si existe, empieza en LBA = 3 + KS_COUNT (donde `KS_COUNT` es el número de sectores que ocupa el kernel y lo escribe `stage2` en el header).

- Flujo de `module_probe()` (resumido):
  1. Calcular LBA del módulo: lee `KS_COUNT` desde el header que dejó `stage2` y calcula `mod_lba = 3 + KS_COUNT`.
  2. Leer el primer sector (probe): realiza una lectura ATA/LBA del `mod_lba` y guarda el sector en un buffer temporal (`g_module_probe_sector`).
  3. Comprobar cabecera ELF: valida `e_ident` (`0x7F 'E' 'L' 'F'`), que sea ELF32 little-endian (`EI_CLASS == 1`, `EI_DATA == 1`), que `e_machine == EM_386`, y que la tabla de program headers tenga un tamaño coherente.
  4. Si la cabecera no es válida, el probe falla y el kernel muestra "NO" en la UI.

- Carga y ejecución (si se acepta):
  - `module_finalize_after_bar()` vuelve a leer todos los sectores declarados para el módulo (`MODULE_SECTORS_COUNT` y `MODULE_SIZE_BYTES` se generan en build) y los copia en RAM temporal.
  - Recorre cada program header `PT_LOAD` y copia la región `[p_offset, p_offset + p_filesz)` a la dirección física `p_paddr` (o `p_vaddr` si `p_paddr` es cero). Se rellenan con cero los bytes extra hasta `p_memsz`.
  - Se comprueba que `e_entry` caiga dentro de alguno de los segmentos cargados; solo entonces se marca el módulo como listo (`STAGE2_MODULE_OK = 1`) y se expone la entrada mediante `module_get_entry()`.
  - Si cualquier paso falla (lectura ATA, cabecera incoherente, `e_entry` fuera de segmento, etc.) se limpian los flags y no se ejecuta el módulo.

- Ejemplos de casos:
  - Módulo ausente: primer sector no contiene `0x7FELF` → el probe falla (salida: "NO").
  - Módulo válido ELF: cabecera correcta y `e_entry` dentro de un `PT_LOAD` → la UI muestra "Cabecera ELF: OK" y ofrece ejecutarlo.
  - Módulo corrupto o truncado: aunque haya cabecera, si la tabla de program headers cae fuera del tamaño real del archivo o un segmento reclama más bytes de los disponibles, `module_finalize_after_bar()` aborta.

- Riesgos y mitigaciones:
  - Si `KS_COUNT` es incorrecto, el LBA calculado será erróneo → probe leerá basura. Mitigación: mantener `stage2` y su header sincronizados con el kernel y considerar checksum.
  - Si `IMAGE_SIZE` es demasiado pequeño y la imagen fue truncada, el módulo puede estar incompleto. Por eso el Makefile amplía automáticamente la imagen si el contenido concatenado excede `IMAGE_SIZE`.
  - Se valida que `e_entry` caiga dentro de un segmento cargado para evitar saltar a memoria basura.

Dónde mirar en el código:
- `kernel_boot/kernelfunciones.c` — implementación de `module_probe()`, `module_finalize_after_bar()` y `ata_read28()`.
- `kernel_boot/kernel.h` — macros como `MODULE_LOAD_ADDRESS`, `MODULE_SECTORS_COUNT` y offsets del header que escribe `stage2`.
- `boot/stage2.asm` — escribe el header con `KS_COUNT` (kernel sectors) que `module_probe()` usa para calcular LBA.
- `kernel_post/` — ejemplo de módulo ELF (`module_entry.c`, `modlib.c`, `module.ld`).

## Direcciones y convenciones importantes

- `stage2` load address: 0x7E00 (ejecución en modo real) — el header se escribe en 0x7E00.
- `kernel` link/load address: 0x10000 (definido en `linker.ld`).
- `MODULE_LOAD_ADDRESS`: 0x00120000 — donde se coloca y ejecuta el módulo post-boot.
- Zona compartida entre stage2 y kernel: `STAGE2_BASE = 0x7E00`. El pseudokernel lee el magic/versión/ks_count desde ahí, es donde debería estar el kernel real.

- De haber errores por solapamiento de código, tocaría mover las direcciones de memoria a otras donde no se lee algo indebido.

## Cómo construir y probar

Recomendación desde la raíz `week2/Sistema_Operativo`:

```
make clean
make
make run   # lanza qemu con os-image.bin
```

- `make` genera `stage1.bin` (512B), `stage2.bin` (1024B), compila y empaqueta el kernel (`kernel.bin`) y el módulo (`module_post.elf` + `module_post.img`) y concatena `os-image.bin`.
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
  - Asegúrate de que el `module_post.img` esté ubicado inmediatamente después del kernel en la imagen (Makefile empaqueta así).
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
Sector N+1..M: module_post.img (MODULE_SECTORS_COUNT sectores)
                                             <- módulo post-boot ELF (si existe)
Resto:      espacio libre (cálculo automático).
------------------------------------------------------------

LBA (para lectura por ATA) usado internamente:
- kernel LBA base = 3 (0-based) -> corresponde a sector físico 4 (1-based)
- módulo LBA base = 3 + KS_COUNT

```

Nota: el kernel se carga en 0x10000 y el módulo se coloca temporalmente en `MODULE_LOAD_ADDRESS` = 0x00120000.

## Ejemplos de salida del kernel (casos típicos)

Los siguientes ejemplos son salidas simuladas de la UI del kernel (`kmain`) para ayudar a entender cómo se ve cada caso. Las líneas importantes están marcadas.

1) Arranque normal, sin módulo disponible

```
               PseudoKernel
      Virus Payal OS Bootloader by Emefedez
=========================================
  Modulo opcional: 0x00120000 NO

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

Explicación: `module_probe()` no encontró cabecera ELF válida (`0x7FELF`) en el LBA esperado. El kernel informa que no hay módulo y continúa.

2) Arranque con módulo válido (sondeado y ejecutable)

```
               PseudoKernel
      Virus Payal OS Bootloader by Emefedez
=========================================
  Modulo opcional: 0x00120000 SONDEADO [ OK ]
  [---====------] (barra de progreso)
  Copia modulo: OK
  -- Detalles modulo --
  Cabecera ELF: OK entry=0x00120000

  (Pulsa cualquier tecla para ejecutar el modulo)

  ESP antes: 0x0008F000
  ... (salida del modulo) ...
  ESP despues: 0x0008F000
  Retorno modulo

  [Modulo finalizado - 3s]

  PseudoKernel
  Modulo opcional:     0x00120000 CARGADO
  ... (resto de info) ...
```

Explicación: `module_probe()` detectó una cabecera ELF válida, `module_finalize_after_bar()` cargó los segmentos `PT_LOAD` y el kernel puede ejecutar la `e_entry` del módulo.

3) Arranque con módulo presente pero corrupto/inválido

```
               PseudoKernel
      Virus Payal OS Bootloader by Emefedez
=========================================
  Modulo opcional: 0x00120000 SONDEADO
  Copia modulo: FALLO
  -- Detalles modulo --
  Cabecera: NO HEADER
  Dump[0..15]: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

  Entry invalido: no se ejecuta modulo

  [Modulo finalizado - 3s]

  PseudoKernel
  Modulo opcional:     0x00120000 (carga incompleta)
  ... (resto de info) ...
```

Explicación: el primer sector del módulo no contiene el magic esperado; el kernel marca fallo y no ejecuta el módulo.

## Ejemplo rápido de depuración (qué mirar cuando algo falla)

- Si el kernel muestra "Stage2: no header detectado": comprobar que `stage2` haya impreso `OK` tras leer el kernel y que `KS_COUNT` es correcto. Verifica `stage2.bin` y `kernel_sects.inc`.
- Si `module_probe()` devuelve falsos negativos: comprobar LBA calculado = `3 + KS_COUNT`. Puedes forzar `KS_COUNT` variable en `kernel_sects.inc` para pruebas.
- Si el módulo se detecta pero la ejecución falla: valida `entry_off` dentro de `[0, sector_size)` y que el módulo esté enlazado para `MODULE_LOAD_ADDRESS`.


