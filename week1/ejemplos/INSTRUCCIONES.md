# 🚀 INSTRUCCIONES DE USO - Programas de Ejemplo Simplez-13

## 📁 Contenido de la Carpeta

Esta carpeta contiene 10 programas de ejemplo completos y listos para ejecutar:

```
ejemplos/
├── README.md                      # Descripción general de todos los programas
├── INSTRUCCIONES.md              # Este archivo - Cómo usar los programas
├── 01_programa_basico.c          # Operaciones básicas
├── 02_programa_contador.c        # Contador con bucles
├── 03_programa_suma_array.c      # Suma de arrays
├── 04_programa_indirecto.c       # Direccionamiento indirecto
├── 05_programa_factorial.c       # Cálculo de factorial
├── 06_programa_maximo.c          # Búsqueda del máximo
├── 07_programa_copia_memoria.c   # Copia de bloques
├── 08_programa_fibonacci.c       # Secuencia Fibonacci
├── 09_programa_debug.c           # Todos los modos de direccionamiento
└── 10_programa_subrutina.c       # Simulación de subrutinas
```

---

## 🔧 Cómo Usar los Programas

### Método 1: Reemplazar main.c (Recomendado)

1. **Elegir un programa:**
   ```bash
   cd ASOC2025/Exercices/ejemplos
   ```

2. **Copiar al directorio principal:**
   ```bash
   cp 01_programa_basico.c ../main.c
   ```

3. **Compilar:**
   ```bash
   cd ..
   make clean
   make
   ```

4. **Ejecutar:**
   ```bash
   ./emulador
   ```
   
   Presiona **Enter** para avanzar paso a paso

---

### Método 2: Compilar Directamente

Puedes compilar cada programa individualmente:

```bash
cd ASOC2025/Exercices

# Compilar instrucciones.c (común a todos)
gcc -c instrucciones.c -o instrucciones.o

# Compilar un programa específico
gcc -c ejemplos/01_programa_basico.c -o main.o

# Enlazar
gcc instrucciones.o main.o -o emulador_basico

# Ejecutar
./emulador_basico
```

---

### Método 3: Script Rápido

Crea un script `run_example.sh`:

```bash
#!/bin/bash
# Script para ejecutar ejemplos rápidamente

if [ -z "$1" ]; then
    echo "Uso: ./run_example.sh [número del 1 al 10]"
    echo "Ejemplo: ./run_example.sh 1"
    exit 1
fi

PROGRAM="ejemplos/0${1}_*.c"
cp $PROGRAM main.c
make clean
make
./emulador
```

Usar:
```bash
chmod +x run_example.sh
./run_example.sh 1    # Ejecuta programa 1
./run_example.sh 5    # Ejecuta programa 5
```

---

## 📚 Orden Recomendado de Aprendizaje

### Nivel Básico (Empezar aquí)
1. **Programa 01** - Operaciones Básicas
   - LD #, ST, ADD directo
   - Flujo básico de la CPU

2. **Programa 02** - Contador
   - Bucles con BZ y BR
   - SUB #, control de flujo

### Nivel Intermedio
3. **Programa 03** - Suma de Array
   - Direccionamiento indexado
   - Recorrido de estructuras

4. **Programa 04** - Direccionamiento Indirecto
   - Punteros en bajo nivel
   - Indirección e indirección indexada

5. **Programa 06** - Búsqueda del Máximo
   - Algoritmos de comparación
   - Actualización condicional

6. **Programa 07** - Copia de Memoria
   - Operaciones en bloque
   - Gestión de múltiples índices

### Nivel Avanzado
7. **Programa 05** - Factorial
   - Bucles anidados
   - Multiplicación mediante sumas

8. **Programa 08** - Fibonacci
   - Secuencias recursivas iterativas
   - Gestión de estado complejo

9. **Programa 10** - Subrutinas
   - Llamadas y retornos
   - Simulación de stack

### Nivel Referencia
10. **Programa 09** - Demo Completa
    - Todos los modos de direccionamiento
    - Referencia rápida

---

## 🎮 Controles Durante la Ejecución

- **Enter**: Avanza al siguiente ciclo de CPU
- **Ctrl+C**: Termina la ejecución
- **Observa**: 
  - Instrucción actual
  - Valores de AC, X, PC
  - Flags Z y H
  - Cálculo de Dirección Efectiva (EA)

---

## 🐛 Debug y Comprensión

Cada programa imprime información de debug en cada ciclo:

```
DEBUG: PC:000 | Inst:A85 | OP: LD # (R:0, M:2, CD:05) | EA/Dato: 005
       -> Estado: PC:001, AC:005, X:000, Z_flag:0, H_flag:0
```

**Interpretación:**
- `PC:000` - Contador de programa (dirección actual)
- `Inst:A85` - Instrucción en hexadecimal
- `OP: LD #` - Operación decodificada
- `R:0` - Registro (0=AC, 1=X)
- `M:2` - Modo de direccionamiento (0-3)
- `CD:05` - Campo de dirección
- `EA/Dato: 005` - Dirección efectiva o dato inmediato
- Estado después de ejecutar la instrucción

---

## 📊 Tabla Rápida de Referencia

### Códigos de Operación (CO)

| Binario | Dec | Nemónico | Descripción |
|---------|-----|----------|-------------|
| 000     | 0   | ST       | Store (Almacenar) |
| 001     | 1   | LD       | Load (Cargar) |
| 010     | 2   | ADD      | Add (Sumar) |
| 011     | 3   | BR       | Branch (Salto incondicional) |
| 100     | 4   | BZ       | Branch if Zero (Salto si cero) |
| 101     | 5   | LD #     | Load Immediate (Carga inmediata) |
| 110     | 6   | SUB #    | Subtract Immediate (Resta inmediata) |
| 111     | 7   | HALT     | Halt (Detener) |

### Modos de Direccionamiento (J/I)

| Bits | Modo | Notación | EA Cálculo | Uso |
|------|------|----------|------------|-----|
| 00   | Directo | `/dir` | EA = CD | Variables simples |
| 01   | Indirecto | `[/dir]` | EA = (CD) | Punteros |
| 10   | Indexado | `/dir[.X]` | EA = CD + X | Arrays |
| 11   | Indirecto Indexado | `[/dir][.X]` | EA = (CD) + X | Arrays de punteros |

---

## 💡 Tips y Trucos

### 1. Modificar Valores
Cambia los valores en los arrays `example_program[]` para experimentar:
```c
/* 000 */ 0xA85, // LD.A, #5    <- Cambia el 5 a otro valor
```

### 2. Agregar Trazas
Añade `printf()` en puntos clave para ver valores específicos:
```c
printf("Valor actual: %d\n", mem[0x20]);
```

### 3. Pausar en Puntos Específicos
Modifica la función `loop()` para pausar en ciertas condiciones:
```c
if (pc == 0x05) {
    printf("¡Punto de interrupción en 0x05!\n");
    getchar();
}
```

### 4. Ver Memoria Completa
Al final del main, añade un dump de memoria:
```c
printf("\nDump de memoria (0x00-0x40):\n");
for (int i = 0; i < 0x40; i++) {
    if (i % 8 == 0) printf("\n%03X: ", i);
    printf("%04X ", mem[i]);
}
```

---

## ❓ Solución de Problemas

### Error: "undefined reference to instrucciones"
**Solución:** Asegúrate de compilar `instrucciones.c`:
```bash
make clean
make
```

### El programa no hace nada
**Solución:** Verifica que `status.h = 0` al inicio y que hay una instrucción HALT.

### Valores incorrectos
**Solución:** Revisa los modos de direccionamiento. El más común es confundir:
- Directo (`/20`) vs Indirecto (`[/20]`)
- Indexado (`/20[.X]`) vs Indirecto Indexado (`[/20][.X]`)

### Bucle infinito
**Solución:** Verifica que:
- El contador se decrementa correctamente
- La condición BZ se evalúa
- Hay un camino hacia HALT

---

## 🎓 Ejercicios Propuestos

Una vez que domines los ejemplos, intenta crear tus propios programas:

1. **Resta de dos números** (usa SUB # y comparación)
2. **Búsqueda del mínimo** (inverso del programa 06)
3. **Inversión de array** (intercambiar array[i] con array[n-i])
4. **Producto de dos números** (multiplicación mediante sumas)
5. **Número primo** (verificar si un número es primo)
6. **Ordenamiento burbuja** (ordenar un array pequeño)
7. **Potencia** (calcular x^n)
8. **Suma de dígitos** (sumar dígitos de un número)

---

## 📖 Recursos Adicionales

- **Emulador Simplez13.pdf** - Documentación oficial completa
- **simplez.h** - Definiciones de la arquitectura
- **instrucciones.c** - Implementación del repertorio
- **main.c** - Programa actual en ejecución

---

## 🤝 Contribuciones

Si creas programas nuevos o mejoras:

1. Usa la misma estructura de los ejemplos
2. Añade comentarios claros
3. Incluye descripción y objetivo
4. Verifica que funcione con `make && ./emulador`
5. Documenta el algoritmo

---

## ⚡ Quick Start (TL;DR)

```bash
cd ASOC2025/Exercices
cp ejemplos/01_programa_basico.c main.c
make clean && make
./emulador
# Presiona Enter repetidamente para ver la ejecución
```

---

**Team Virus Payal** 🖤  
*"El hardware no miente, solo ejecuta. Aprende a hablar su idioma."*

---

¿Preguntas? Revisa el **README.md** en esta carpeta para descripciones detalladas de cada programa.