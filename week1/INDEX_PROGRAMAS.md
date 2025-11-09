# 📑 ÍNDICE RÁPIDO DE PROGRAMAS SIMPLEZ-13

## 🎯 Acceso Rápido

| # | Nombre | Dificultad | Tiempo | Conceptos Clave |
|---|--------|------------|--------|-----------------|
| 01 | [Operaciones Básicas](#01-operaciones-básicas) | ⭐ | 5-10 min | LD#, ST, ADD |
| 02 | [Contador](#02-contador-ascendente) | ⭐⭐ | 10-15 min | Bucles, BZ, SUB# |
| 03 | [Suma Array](#03-suma-de-array) | ⭐⭐⭐ | 15-20 min | Indexado, Arrays |
| 04 | [Indirecto](#04-direccionamiento-indirecto) | ⭐⭐⭐ | 20-25 min | Punteros |
| 05 | [Factorial](#05-cálculo-de-factorial) | ⭐⭐⭐⭐ | 25-30 min | Bucles anidados |
| 06 | [Máximo](#06-búsqueda-del-máximo) | ⭐⭐⭐ | 15-20 min | Comparación |
| 07 | [Copia Memoria](#07-copia-de-bloques) | ⭐⭐⭐ | 15-20 min | Transferencia |
| 08 | [Fibonacci](#08-secuencia-fibonacci) | ⭐⭐⭐⭐ | 25-30 min | Secuencias |
| 09 | [Demo Completa](#09-demo-de-todos-los-modos) | ⭐⭐⭐ | 20-30 min | Referencia |
| 10 | [Subrutinas](#10-simulación-de-subrutinas) | ⭐⭐⭐⭐⭐ | 30-40 min | CALL/RET |

---

## 📖 Descripciones Detalladas

### 01. Operaciones Básicas
```
Archivo: 01_programa_basico.c
Entrada: 5, 3
Salida: 8 (en mem[0x22])
Operación: 5 + 3 = 8
```
**¿Qué aprenderás?**
- Cargar valores inmediatos
- Almacenar en memoria
- Sumar valores
- Flujo básico de CPU

**Ejecutar:**
```bash
./run_example.sh 1
```

---

### 02. Contador Ascendente
```
Archivo: 02_programa_contador.c
Entrada: Contador de 10 a 1
Salida: 55 (suma 1+2+...+10)
Operación: Σ(1 hasta 10)
```
**¿Qué aprenderás?**
- Implementar bucles
- Saltos condicionales (BZ)
- Saltos incondicionales (BR)
- Decrementar contadores

**Ejecutar:**
```bash
./run_example.sh 2
```

---

### 03. Suma de Array
```
Archivo: 03_programa_suma_array.c
Entrada: [10, 20, 30, 40, 50]
Salida: 150
Operación: Suma de elementos
```
**¿Qué aprenderás?**
- Direccionamiento indexado
- Recorrer arrays
- Usar registro X como índice
- Acumulación de valores

**Ejecutar:**
```bash
./run_example.sh 3
```

---

### 04. Direccionamiento Indirecto
```
Archivo: 04_programa_indirecto.c
Entrada: Punteros y arrays
Salida: Valores accedidos indirectamente
Operación: Demostración de *ptr y ptr[i]
```
**¿Qué aprenderás?**
- Punteros en bajo nivel
- Indirección simple [/dir]
- Indirección indexada [/dir][.X]
- Acceso indirecto a datos

**Ejecutar:**
```bash
./run_example.sh 4
```

---

### 05. Cálculo de Factorial
```
Archivo: 05_programa_factorial.c
Entrada: 5
Salida: 120 (5!)
Operación: 5 × 4 × 3 × 2 × 1
```
**¿Qué aprenderás?**
- Bucles anidados
- Multiplicación por sumas
- Gestión de múltiples variables
- Algoritmos iterativos complejos

**Ejecutar:**
```bash
./run_example.sh 5
```

---

### 06. Búsqueda del Máximo
```
Archivo: 06_programa_maximo.c
Entrada: [15, 42, 8, 99, 23]
Salida: 99
Operación: Encontrar máximo
```
**¿Qué aprenderás?**
- Algoritmos de búsqueda
- Comparación mediante resta
- Actualización condicional
- Iterar sobre colecciones

**Ejecutar:**
```bash
./run_example.sh 6
```

---

### 07. Copia de Bloques
```
Archivo: 07_programa_copia_memoria.c
Entrada: [11, 22, 33, 44, 55]
Salida: Copia en otra zona
Operación: memcpy(destino, origen, 5)
```
**¿Qué aprenderás?**
- Transferencia de datos
- Indexación dual
- Gestión de buffers
- Operaciones en bloque

**Ejecutar:**
```bash
./run_example.sh 7
```

---

### 08. Secuencia Fibonacci
```
Archivo: 08_programa_fibonacci.c
Entrada: n = 8
Salida: [0, 1, 1, 2, 3, 5, 8, 13]
Operación: fib[i] = fib[i-1] + fib[i-2]
```
**¿Qué aprenderás?**
- Secuencias matemáticas
- Dependencias entre valores
- Variables temporales
- Estado iterativo complejo

**Ejecutar:**
```bash
./run_example.sh 8
```

---

### 09. Demo de Todos los Modos
```
Archivo: 09_programa_debug.c
Entrada: Varios valores de prueba
Salida: Demostración de cada modo
Operación: Referencia completa
```
**¿Qué aprenderás?**
- Directo (/dir)
- Indirecto ([/dir])
- Indexado (/dir[.X])
- Indirecto Indexado ([/dir][.X])
- Inmediato (#valor)

**Ejecutar:**
```bash
./run_example.sh 9
```

---

### 10. Simulación de Subrutinas
```
Archivo: 10_programa_subrutina.c
Entrada: 5, 3
Salida: 16 (doble(suma(5,3)))
Operación: Llamadas anidadas
```
**¿Qué aprenderás?**
- Simular CALL/RET
- Direcciones de retorno
- Saltos indirectos
- Modularidad del código

**Ejecutar:**
```bash
./run_example.sh 10
```

---

## 🎓 Rutas de Aprendizaje

### 🛤️ Ruta 1: Principiante Total
```
1 → 2 → 3 → 9 → Crea tu propio programa
```
**Duración:** 2-3 horas  
**Objetivo:** Familiarizarse con lo básico

---

### 🛤️ Ruta 2: Estudiante Intermedio
```
1 → 2 → 3 → 4 → 6 → 7 → 9 → Ejercicios propuestos
```
**Duración:** 4-5 horas  
**Objetivo:** Dominar direccionamiento

---

### 🛤️ Ruta 3: Avanzado
```
1 → 2 → 3 → 4 → 5 → 8 → 10 → Proyecto complejo
```
**Duración:** 6-8 horas  
**Objetivo:** Algoritmos complejos

---

### 🛤️ Ruta 4: Completista
```
1 → 2 → 3 → 4 → 6 → 7 → 9 → 5 → 8 → 10
```
**Duración:** 8-10 horas  
**Objetivo:** Maestría completa

---

## 🔍 Búsqueda por Concepto

### Quiero aprender sobre...

**Bucles:**
- Programa 02 (básico)
- Programa 03 (con array)
- Programa 05 (anidados)

**Arrays:**
- Programa 03 (suma)
- Programa 06 (búsqueda)
- Programa 07 (copia)

**Punteros:**
- Programa 04 (indirecto)
- Programa 09 (referencia)

**Algoritmos:**
- Programa 05 (factorial)
- Programa 06 (máximo)
- Programa 08 (fibonacci)

**Conceptos Avanzados:**
- Programa 10 (subrutinas)
- Programa 05 (bucles anidados)

---

## 📊 Matriz de Conceptos

|  | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| LD # | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| ST | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| ADD | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| SUB # | | ✓ | ✓ | | ✓ | ✓ | ✓ | ✓ | ✓ | |
| BR | | ✓ | ✓ | | ✓ | ✓ | ✓ | ✓ | | ✓ |
| BZ | | ✓ | ✓ | | ✓ | ✓ | ✓ | ✓ | | |
| Directo | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Indirecto | | | | ✓ | | | | | ✓ | ✓ |
| Indexado | | | ✓ | | | ✓ | ✓ | ✓ | ✓ | |
| Ind+Idx | | | | ✓ | | | | | ✓ | |
| Bucles | | ✓ | ✓ | | ✓ | ✓ | ✓ | ✓ | | |
| Arrays | | | ✓ | ✓ | | ✓ | ✓ | ✓ | ✓ | |
| Punteros | | | | ✓ | | | | | ✓ | ✓ |

---

## 🚀 Comandos Rápidos

```bash
# Ejecutar programa específico
./run_example.sh [1-10]

# Compilar manualmente
cp ejemplos/01_programa_basico.c main.c
make clean && make
./emulador

# Ver ayuda del script
./run_example.sh

# Listar todos los programas
ls -1 ejemplos/*.c
```

---

## 📚 Documentación Adicional

- **README.md** - Descripciones completas de cada programa
- **INSTRUCCIONES.md** - Guía de uso detallada
- **RESUMEN.md** - Visión general del proyecto

---

## 💾 Estructura de Archivos

```
ASOC2025/Exercices/
│
├── ejemplos/              # Esta carpeta
│   ├── 01_*.c ... 10_*.c # 10 programas
│   ├── README.md         # Descripciones
│   ├── INSTRUCCIONES.md  # Guía de uso
│   ├── RESUMEN.md        # Resumen completo
│   └── INDEX_PROGRAMAS.md # Este archivo
│
├── simplez.h             # Definiciones hardware
├── instrucciones.c       # Implementación
├── main.c                # Programa actual
├── Makefile              # Compilación
└── run_example.sh        # Script ejecutor
```

---

## 🎯 Objetivos por Programa

| Programa | Objetivo Principal |
|----------|-------------------|
| 01 | Entender el flujo básico |
| 02 | Dominar bucles y control de flujo |
| 03 | Trabajar con arrays |
| 04 | Comprender punteros |
| 05 | Implementar algoritmos complejos |
| 06 | Crear algoritmos de búsqueda |
| 07 | Gestionar bloques de memoria |
| 08 | Manejar secuencias matemáticas |
| 09 | Referencia de todos los modos |
| 10 | Simular características avanzadas |

---

**Team Virus Payal** 🖤  
*Quick reference for assembly mastery*

**Última actualización:** Noviembre 2024  
**Versión:** 1.0