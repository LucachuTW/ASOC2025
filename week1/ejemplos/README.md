# 📚 Programas de Ejemplo para Simplez-13

Esta carpeta contiene múltiples programas de ejemplo para el emulador Simplez-13, cada uno diseñado para demostrar diferentes características y conceptos de la arquitectura.

---

## 📋 Lista de Programas

### 1. **programa_basico.c** - Operaciones Básicas
**Descripción:** Introduce las operaciones fundamentales de Simplez-13.
- Carga de valores inmediatos con `LD #`
- Almacenamiento en memoria con `ST`
- Suma de valores con `ADD`
- Modo de direccionamiento directo

**Concepto clave:** Familiarización con el flujo básico: cargar → operar → almacenar.

---

### 2. **programa_contador.c** - Contador Ascendente
**Descripción:** Implementa un contador que suma de 1 hasta 10.
- Uso de bucles con `BZ` (Branch if Zero)
- Operación de resta inmediata con `SUB #`
- Control de flujo condicional
- Actualización del flag Z

**Concepto clave:** Estructuras de control iterativas y flags de estado.

---

### 3. **programa_suma_array.c** - Suma de Array
**Descripción:** Suma todos los elementos de un array en memoria.
- Direccionamiento indexado (uso del registro X)
- Recorrido de estructuras de datos
- Acumulación de resultados
- Bucles con contador

**Concepto clave:** Procesamiento de arrays y direccionamiento indexado.

---

### 4. **programa_indirecto.c** - Direccionamiento Indirecto
**Descripción:** Demuestra el uso de punteros en Simplez-13.
- Direccionamiento indirecto simple `[/dir]`
- Direccionamiento indirecto indexado `[/dir][.X]`
- Manipulación de punteros
- Acceso indirecto a datos

**Concepto clave:** Conceptos de punteros y direccionamiento indirecto.

---

### 5. **programa_factorial.c** - Cálculo de Factorial
**Descripción:** Calcula el factorial de un número pequeño (ej: 5! = 120).
- Multiplicación mediante sumas repetidas
- Bucles anidados
- Control de flujo complejo
- Uso combinado de AC y X

**Concepto clave:** Algoritmos iterativos y operaciones complejas.

---

### 6. **programa_maximo.c** - Búsqueda del Máximo
**Descripción:** Encuentra el valor máximo en un array de números.
- Comparación mediante resta
- Decisiones basadas en el flag Z
- Actualización condicional de variables
- Recorrido de arrays

**Concepto clave:** Algoritmos de búsqueda y comparación.

---

### 7. **programa_copia_memoria.c** - Copia de Bloques
**Descripción:** Copia un bloque de memoria de una zona a otra.
- Operaciones de memoria en bloque
- Direccionamiento indexado para origen y destino
- Manipulación de múltiples índices
- Transferencia de datos

**Concepto clave:** Operaciones de memoria y gestión de datos.

---

### 8. **programa_fibonacci.c** - Secuencia de Fibonacci
**Descripción:** Genera los primeros N números de la secuencia de Fibonacci.
- Variables temporales en memoria
- Intercambio de valores
- Secuencias matemáticas
- Almacenamiento de resultados

**Concepto clave:** Secuencias recursivas iterativas y gestión de estado.

---

### 9. **programa_debug.c** - Demostración de Todos los Modos
**Descripción:** Programa didáctico que usa todos los modos de direccionamiento.
- Directo: `/dir`
- Indirecto: `[/dir]`
- Indexado: `/dir[.X]`
- Indirecto Indexado: `[/dir][.X]`
- Inmediato: `#valor`

**Concepto clave:** Referencia completa de todos los modos de direccionamiento.

---

### 10. **programa_subrutina.c** - Simulación de Subrutinas
**Descripción:** Simula llamadas a subrutinas usando saltos y una pila simple.
- Saltos incondicionales con `BR`
- Simulación de stack
- Retorno de subrutinas
- Estructura modular del código

**Concepto clave:** Conceptos de subrutinas y organización modular.

---

## 🚀 Cómo Usar los Programas

1. **Copiar el contenido** del programa deseado al archivo `main.c` principal
2. **Compilar** con `make`:
   ```bash
   make clean
   make
   ```
3. **Ejecutar** el emulador:
   ```bash
   ./emulador
   ```
4. **Presionar Enter** para avanzar paso a paso y ver la ejecución

---

## 📖 Estructura de los Programas

Todos los programas siguen esta estructura:

```c
#include <stdio.h>
#include <string.h>
#include "simplez.h"

// Definición de variables globales (del .h)
uint16_t mem[MEM_SIZE];
uint16_t acc;
uint16_t x;
uint16_t pc;
struct status status;

// Array de funciones y nombres...

void loop() {
    // Ciclo Fetch-Decode-Execute
}

int main() {
    // 1. Definir el programa en hexadecimal
    uint16_t example_program[] = { ... };
    
    // 2. Cargar en memoria
    memset(mem, 0, sizeof(mem));
    memcpy(mem, example_program, sizeof(example_program));
    
    // 3. Inicializar CPU
    pc = 0; acc = 0; x = 0;
    status.z = 0; status.h = 0;
    
    // 4. Ejecutar
    loop();
    
    // 5. Mostrar resultados
    printf("Estado final...\n");
    
    return 0;
}
```

---

## 🎓 Recursos Adicionales

- **Emulador Simplez13.pdf** - Documentación completa de la arquitectura
- **simplez.h** - Definiciones de hardware y funciones
- **instrucciones.c** - Implementación del repertorio de instrucciones

---

## 💡 Tips para Aprender

1. **Empieza por el básico** - Entiende el flujo fundamental antes de avanzar
2. **Lee el debug** - Los mensajes muestran el estado en cada ciclo
3. **Modifica valores** - Cambia números en los programas y observa el comportamiento
4. **Experimenta** - Crea tus propios programas combinando conceptos
5. **Consulta el PDF** - La documentación oficial es tu mejor amigo

---

## 🔧 Formato de Instrucciones

Cada instrucción de 12 bits tiene esta estructura:

```
[11-9] [8] [7-6] [5-0]
  CO    R   J/I    CD

CO  = Código de Operación (3 bits)
R   = Registro (0=AC, 1=X)
J/I = Modo de Direccionamiento (2 bits)
CD  = Campo de Dirección (6 bits)
```

---

**Team Virus Payal** 🖤  
*Learn by doing. Break to understand. Build to defend.*