# kernel_post (Modulo(s) post-boot)

Este directorio está pensado para contener módulos opcionales cargados por `stage2` **después** del kernel principal.

## Dirección de carga

El módulo actual (si existe) se carga en memoria física `0x12000` (segmento `0x1200`). Esta dirección se define en `kernel_boot/kernel.h` como `MODULE_LOAD_ADDRESS`.

## Formato mínimo sugerido

Por ahora `stage2` sólo lee 1 sector (512 bytes) adicional tras el kernel y marca el flag `MODULE_OK` si la lectura devuelve éxito. No se valida contenido.

Puedes definir un encabezado simple:

Offset | Tamaño | Descripción
------ | ------ | -----------
0x00   | 4      | Magic (por ejemplo "MOD0")
0x04   | 2      | Versión
0x06   | 2      | Longitud útil (bytes)
0x08   | n      | Datos / código

## Futuras mejoras posibles

- Validar magic y versión para mostrar en la UI del kernel.
- Cargar más de 1 sector dinámicamente (añadir recuento en stage2).
- Pasar un descriptor estructurado al kernel (por ejemplo en memoria compartida antes de 0x7E00).
- Implementar firma / checksum independiente del módulo.

## Construcción de un módulo

Actualmente no hay regla Makefile. Puedes generar un binario crudo de 512 bytes y concatenarlo tras el kernel manualmente para pruebas:

1. Crear archivo `module.bin` (máx 512 bytes para la prueba actual).
2. Insertarlo antes del truncado de la imagen o ajustar el Makefile para añadirlo entre `kernel.bin` y el truncado.

Ejemplo (manual, fish shell):
```
cp os-image.bin os-image-con-mod.bin
dd if=module.bin of=os-image-con-mod.bin bs=512 seek=$((3 + $(stat -f%z kernel.bin) / 512 )) conv=notrunc
```

> Nota: El cálculo exacto del sector base del módulo es `4 + KS_COUNT - 1` si el kernel empieza en el sector 3 (dependiendo del layout actual). Revisa el Makefile antes de automatizar.

## Integración futura

Cuando se extienda, el kernel podrá:

1. Leer encabezado del módulo en `MODULE_LOAD_ADDRESS`.
2. Validar magic y tamaño.
3. Ejecutar código (saltando a una entrada) o exponer servicios.

---
Contribuciones bienvenidas.
