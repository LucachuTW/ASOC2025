# kernel_post (Módulos post-boot)

Este directorio contiene módulos opcionales que el stage2 ha de copiar a memoria y que el pseudokernel puede ejecutar tras terminar su arranque.

## Dirección de carga
El módulo se carga en memoria física en 0x00120000. Esta dirección está definida como `MODULE_LOAD_ADDRESS` en `kernel_boot/kernel.h` y es la base que usa el script de enlace (`module.ld`) para situar los segmentos `PT_LOAD`.

## Formato del módulo
El módulo se entrega como ELF32 (`EM_386`, little-endian). El pseudokernel comprueba la cabecera estándar `0x7FELF`, valida los program headers y copia uno a uno los segmentos `PT_LOAD` a la dirección física indicada por `p_paddr` (o `p_vaddr` si `p_paddr == 0`). Tras copiar, limpia con ceros la zona `p_memsz - p_filesz` y sólo permite la ejecución si `e_entry` cae dentro de alguno de los segmentos cargados.

`module_header.S` (legacy) con cabecera `MBIN/MOD0` se conserva en el repositorio por motivos históricos, pero la build actual ya no lo utiliza.

## Qué hace stage2 / kernel
- Stage2: se limita a cargar el pseudokernel, habilitar A20, configurar la GDT y escribir en 0x7E00 el header con `KS_COUNT`. No toca el módulo.
- Kernel: tras dibujar la UI y la barra de progreso, el kernel:
  - Calcula `mod_lba = 3 + KS_COUNT` y lee el primer sector mediante ATA PIO para comprobar la cabecera ELF.
  - Si la cabecera es válida, lee todos los sectores del módulo (`MODULE_SECTORS_COUNT`) a memoria temporal, recorre los program headers `PT_LOAD`, copia cada segmento y rellena con ceros el `bss`.
  - Marca el módulo como cargado (`STAGE2_MODULE_OK = 1`), expone la entrada real con `module_get_entry()` y espera confirmación del usuario antes de saltar a `e_entry`.
  - Si algo falla (lectura ATA, cabecera incoherente, `e_entry` fuera de segmento), los flags quedan a cero y no se ejecuta el módulo.

## Contenido de kernel_post
- `module_entry.c` — demo de módulo; contiene `void module_entry(void)` (punto de entrada).
- `modlib.h` / `modlib.c` — pequeñas utilidades para impresión en VGA/teclado usadas por el módulo.
- `module.ld` — linker script ELF que fija la base en `0x00120000` (coincide con la carga).
- `module_header.S` — versión legacy de cabecera plana (MBIN/MOD0).

## Compatibilidad y notas
- El módulo puede ocupar varios sectores; el `Makefile` genera `module_post.img` alineado a 512 bytes y exporta `MODULE_SECTORS_COUNT`/`MODULE_SIZE_BYTES` en `kernel_boot/module_info.h`.
- `module_header.S` sólo es relevante si quieres estudiar el formato legacy; no participa en la build.
- El `Makefile` ya compila y enlaza `module_post.elf` + `module_post.img` automáticamente en la imagen de prueba (`make` / `make run`).

## Construcción rápida
- Desde la raíz del proyecto:
  - make clean && make
  - make run  (lanza QEMU con la imagen generada)

## Sugerencias
- Añadir firma/checksum en cabecera para seguridad/validación adicional.
- Mover ejemplos antiguos (`module_post.asm` / versiones legacy) a `kernel_post/examples/` si confunden.
