# kernel_post (Módulos post-boot)

Este directorio contiene módulos opcionales que el stage2 ha de copiar a memoria y que el pseudokernel puede ejecutar tras terminar su arranque.

## Dirección de carga
El módulo se carga en memoria física en 0x00012000. Esta dirección está definida como `MODULE_LOAD_ADDRESS` en `kernel_boot/kernel.h` y es la VMA usada al linkar el módulo (`module.ld`).

## Formato de cabecera (actual)
El kernel espera una cabecera al inicio del binario del módulo. El formato actual es:

Offset | Tamaño | Descripción
------ | ------ | -----------
0x00   | 4      | Magic ASCII: `"MBIN"` (se mantiene compatibilidad con `"MOD0"` legacy)
0x04   | 2      | version (u16)
0x06   | 2      | length (u16) — tamaño total del módulo en bytes
0x08   | 4      | entry_off (u32) — offset desde inicio del módulo hacia la función de entrada
0x0C+  | n      | payload (código / datos)

Notas:
- `length` determina cuántos bytes/sectors se han de cargar desde disco. Stage2/Kernel cargan toda la longitud indicada (ceil(length/512) sectores).
- El parser del kernel valida magic y que `entry_off` quede dentro de `[0..length-1]` antes de saltar.

## Qué hace stage2 / kernel
- Stage2: carga la imagen del kernel (varios sectores) y, si existe un sector adicional con módulo, copia el primer sector a `MODULE_LOAD_ADDRESS` para sondear la cabecera.
- Kernel: tras dibujar la UI y la barra, el kernel:
  - Sondea la cabecera en `MODULE_LOAD_ADDRESS`.
  - Valida magic/version/length/entry_off.
  - Calcula los sectores necesarios y carga el resto del módulo desde disco si `length` > 512.
  - Muestra estado (dump de bytes, checksum) y permite ejecutar el módulo si es válido.
  - Salta a `MODULE_LOAD_ADDRESS + entry_off` solo si todas las comprobaciones pasan.

## Contenido de kernel_post
- `module_header.S` — sección/constantes de cabecera y símbolos exportados (ej. `__module_size`, `module_entry`).
- `module_entry.c` — demo de módulo; contiene `void module_entry(void)` (punto de entrada).
- `modlib.h` / `modlib.c` — pequeñas utilidades para impresión en VGA/teclado usadas por el módulo.
- `module.ld` — linker script que fija VMA a `0x00012000` (coincide con la carga).

## Compatibilidad y notas
- Se acepta la cabecera legacy `"MOD0"` por compatibilidad, pero el identificador estándar actual es `"MBIN"`.
- El módulo puede ocupar varios sectores; no debe ser truncado a 512 bytes.
- `entry_off` es relativo al inicio del módulo; el kernel solo ejecuta si está dentro de `length`. Se supone que debería actualizarse automáticamente.
- El Makefile ya compila y enlaza `module_post.bin` automáticamente en la imagen de prueba (`make` / `make run`).

## Construcción rápida
- Desde la raíz del proyecto:
  - make clean && make
  - make run  (lanza QEMU con la imagen generada)

## Sugerencias
- Añadir firma/checksum en cabecera para seguridad/validación adicional.
- Mover ejemplos antiguos (`module_post.asm` / versiones legacy) a `kernel_post/examples/` si confunden.
