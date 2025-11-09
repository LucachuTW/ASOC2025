# Funcionamiento del Bootloader y Módulo Post-Boot

## Estructura general
El sistema se compone de varios segmentos y archivos que trabajan juntos para arrancar el sistema, cargar el kernel y ejecutar un módulo opcional. El flujo es:

1. **stage1.asm** (Bootloader 16 bits, sector 0)
2. **stage2.asm** (Bootloader extendido, sector 1-2)
3. **Kernel** (C, modo protegido, desde sector 3)
4. **Módulo post-boot** (kernel_post, desde el último sector del kernel)

---

## 1. `stage1.asm` (Bootloader inicial)
- **Ubicación:** sector 0 del disco.
- **Función:**
  - Inicializa los registros y el stack.
  - Muestra mensaje de carga.
  - Carga `stage2` (2 sectores) en memoria (0x7E00).
  - Salta a `stage2`.
- **Notas:**
  - Solo usa BIOS (modo real).
  - Si hay error de disco, muestra mensaje y se detiene.

---

## 2. `stage2.asm` (Bootloader extendido)
- **Ubicación:** sectores 1 y 2.
- **Función:**
  - Carga el kernel desde disco (sectores 4+).
  - Marca estado de carga en memoria compartida (0x7E00..0x7E03).
  - Habilita línea A20.
  - Configura la GDT y cambia a modo protegido (32 bits).
  - Salta al kernel en 0x10000.
- **Notas:**
  - Deja indicadores para que el kernel sepa si el módulo opcional está presente.
  - No carga el módulo, solo el kernel.

---

## 3. `kernel_boot/` (PseudoKernel)
- **Archivos clave:**
  - `start.S`: punto de entrada, configura el stack y llama a `kmain`.
  - `kernel.c`: lógica principal, UI, validación HW, carga y ejecución del módulo.
  - `kernelfunciones.c`: utilidades de video, barras, análisis de cabecera, carga ATA, etc.
  - `kernel.h`: definiciones, colores, prototipos.
- **Flujo:**
  1. Muestra banner y estado HW.
  2. Sonda el módulo opcional y dibuja barra de progreso.
  3. Si hay módulo, lo valida, muestra cabecera y dump de bytes.
  4. Espera tecla y ejecuta el módulo (saltando a su entry).
  5. Al volver, muestra estado final y espera.
- **Validaciones:**
  - Verifica si es un módulo "limpio".
  - Carga todos los sectores necesarios según el campo `length` de la cabecera.

---

## 4. `kernel_post/` (Módulos post-boot, la idea es poner aquí el kernel de verdad)
- **Estructura:**
  - `module_header.S`: cabecera del módulo (magic, versión, tamaño, entry).
  - `module_entry.c`: lógica principal del módulo (demo, diagnóstico, UI).
  - `modlib.c`/`modlib.h`: utilidades para el módulo (VGA, formato, teclado).
  - `module.ld`: script de linker, define dirección de carga y símbolos de tamaño.
- **Formato de cabecera:**
  - Magic: "MBIN" (o "MOD0" para legacy)
  - Version: u16
  - Length: u16 (bytes totales del binario)
  - Entry_off: u32 (offset relativo al inicio)
- **Funcionamiento:**
  - El kernel carga el módulo en 0x12000 y salta a la función de entrada.
  - El módulo puede limpiar pantalla, mostrar info, esperar tecla y devolver el control.

---

## 5. `Makefile` y construcción
- Compila cada segmento por separado.
- Empaqueta los binarios en orden: stage1, stage2, kernel, módulo.
- El módulo ya no se trunca a 512 bytes; se empaqueta completo y el kernel lo carga según su tamaño real.
- Para ejecutar:

```fish
make clean
make
make run
```

---

## Resumen de carpetas y archivos
- `boot/`: stage1.asm, stage2.asm
- `kernel_boot/`: start.S, kernel.c, kernelfunciones.c, kernel.h
- `kernel_post/`: module_header.S, module_entry.c, modlib.c, modlib.h, module.ld
- `doc/`: funcionamiento.md (este documento)

---

## Extensiones y mejoras posibles
- Soporte para módulos mayores a 64 KB (cambiar length a u32).
- Validación de checksum en el módulo.
- Carga dinámica de varios módulos.
- Interfaz gráfica o menú de selección de módulo.

---

## Autoría y créditos
- Proyecto y bootloader: Emefedez
- Adaptaciones y modularización: colaborativo

---

## Contacto y contribuciones
Para dudas, mejoras o contribuciones, revisa el README.md principal o abre un issue en el repositorio.
