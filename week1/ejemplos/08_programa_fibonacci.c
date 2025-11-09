#include <stdio.h>
#include <string.h>
#include "simplez.h"

// --- Definición de Variables Globales ---
uint16_t mem[MEM_SIZE];
uint16_t acc;
uint16_t x;
uint16_t pc;
struct status status;

// Array de punteros a funciones para mapear opcodes a funciones
void (*iset[])(uint8_t reg, uint16_t data) = {
    st,         // 000 (0)
    ld,         // 001 (1)
    add,        // 010 (2)
    br,         // 011 (3)
    bz,         // 100 (4)
    ld_sharp,   // 101 (5)
    sub_sharp,  // 110 (6)
    halt        // 111 (7)
};

// Nombres para el depurador
char *iset_names[] = {"ST", "LD", "ADD", "BR", "BZ", "LD #", "SUB #", "HALT"};

void loop() {
    while (!status.h) {
        uint16_t inst = mem[pc] & 0xFFF;
        uint8_t op = (inst >> 9) & 0x7;
        uint8_t reg = (inst >> 8) & 0x1;
        uint8_t dirm = (inst >> 6) & 0x3;
        uint16_t cd = inst & 0x3F;

        uint16_t data;
        if (op == 5 || op == 6) {
            data = cd;
        } else {
            switch (dirm) {
                case 0: data = cd; break;
                case 1: data = mem[cd]; break;
                case 2: data = cd + x; break;
                case 3: data = mem[cd] + x; break;
            }
        }

        printf("DEBUG: PC:%03X | Inst:%03X | OP: %s (R:%d, M:%d, CD:%02X) | EA/Dato: %03X\n", 
                pc, inst, iset_names[op], reg, dirm, cd, data);
        
        iset[op](reg, data);
        pc++;
        
        printf("       -> Estado: PC:%03X, AC:%03X, X:%03X, Z_flag:%d, H_flag:%d\n", 
                pc, acc & 0xFFF, x & 0xFFF, status.z, status.h);
        getchar();
    }
}

int main() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║    PROGRAMA 8: SECUENCIA DE FIBONACCI EN SIMPLEZ-13         ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Generar los primeros 8 números de Fibonacci\n");
    printf("  - Variables temporales en memoria\n");
    printf("  - Intercambio de valores\n");
    printf("  - Secuencias matemáticas iterativas\n");
    printf("  - Almacenamiento de resultados en array\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  fib[0] = 0\n");
    printf("  fib[1] = 1\n");
    printf("  para i = 2 hasta n:\n");
    printf("      fib[i] = fib[i-1] + fib[i-2]\n");
    printf("\n");
    printf("SECUENCIA FIBONACCI: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34...\n");
    printf("RESULTADOS ESPERADOS (primeros 8): [0, 1, 1, 2, 3, 5, 8, 13]\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Generar secuencia de Fibonacci
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        
        // Inicializar primeros dos valores
        /* 000 */ 0xA80, // LD.A, #0         ; AC = 0
        /* 001 */ 0x020, // ST.A, /20        ; fib[0] = 0
        /* 002 */ 0xA81, // LD.A, #1         ; AC = 1
        /* 003 */ 0x021, // ST.A, /21        ; fib[1] = 1
        
        // Inicializar contador (calcular 6 números más: índices 2-7)
        /* 004 */ 0xB86, // LD.X, #6         ; X = 6 (contador de iteraciones)
        /* 005 */ 0x030, // ST.X, /30        ; mem[0x30] = contador
        
        // Inicializar índice del array
        /* 006 */ 0xA82, // LD.A, #2         ; AC = 2 (índice actual)
        /* 007 */ 0x031, // ST.A, /31        ; mem[0x31] = índice
        
        // BUCLE PRINCIPAL: calcular fib[i] = fib[i-1] + fib[i-2]
        /* 008 */ 0x230, // LD.A, /30        ; AC = contador
        /* 009 */ 0x800, // BZ, /00          ; Si contador = 0, terminar
        
        // Calcular índice-1 e índice-2
        /* 00A */ 0x231, // LD.A, /31        ; AC = índice actual
        /* 00B */ 0xC81, // SUB.A, #1        ; AC = índice - 1
        /* 00C */ 0x032, // ST.A, /32        ; mem[0x32] = índice-1
        
        /* 00D */ 0x231, // LD.A, /31        ; AC = índice actual
        /* 00E */ 0xC82, // SUB.A, #2        ; AC = índice - 2
        /* 00F */ 0x033, // ST.A, /33        ; mem[0x33] = índice-2
        
        // Cargar fib[índice-1] usando indexación
        /* 010 */ 0x332, // LD.X, /32        ; X = índice-1
        /* 011 */ 0x280, // LD.A, /00[.X]    ; AC = mem[0 + X] = fib[índice-1]
        /* 012 */ 0x034, // ST.A, /34        ; mem[0x34] = fib[i-1]
        
        // Cargar fib[índice-2]
        /* 013 */ 0x333, // LD.X, /33        ; X = índice-2
        /* 014 */ 0x280, // LD.A, /00[.X]    ; AC = fib[índice-2]
        /* 015 */ 0x434, // ADD.A, /34       ; AC = fib[i-2] + fib[i-1]
        
        // Guardar resultado en fib[índice]
        /* 016 */ 0x331, // LD.X, /31        ; X = índice actual
        /* 017 */ 0x080, // ST.A, /00[.X]    ; mem[0 + X] = fib[índice]
        
        // Incrementar índice
        /* 018 */ 0x231, // LD.A, /31        ; AC = índice
        /* 019 */ 0xA81, // LD.A, #1         ; AC = 1
        /* 01A */ 0x431, // ADD.A, /31       ; AC = índice + 1
        /* 01B */ 0x031, // ST.A, /31        ; índice++
        
        // Decrementar contador
        /* 01C */ 0x230, // LD.A, /30        ; AC = contador
        /* 01D */ 0xC81, // SUB.A, #1        ; AC = contador - 1
        /* 01E */ 0x030, // ST.A, /30        ; contador--
        
        /* 01F */ 0x608, // BR, /08          ; Repetir bucle
        
        /* 020 */ 0xE00, // HALT             ; Detener
        
        // Zona donde se almacena la secuencia Fibonacci (0x20-0x27)
        // Los primeros dos valores se inicializan, el resto se calcula
        /* 021 */ 0x000, // fib[1] (se inicializa a 1)
        /* 022 */ 0x000, // fib[2] (se calculará)
        /* 023 */ 0x000, // fib[3]
        /* 024 */ 0x000, // fib[4]
        /* 025 */ 0x000, // fib[5]
        /* 026 */ 0x000, // fib[6]
        /* 027 */ 0x000, // fib[7]
        
        /* 028 */ 0x000,
        /* 029 */ 0x000,
        /* 02A */ 0x000,
        /* 02B */ 0x000,
        /* 02C */ 0x000,
        /* 02D */ 0x000,
        /* 02E */ 0x000,
        /* 02F */ 0x000,
        /* 030 */ 0x006, // contador (6 iteraciones)
        /* 031 */ 0x002, // índice actual
        /* 032 */ 0x000, // índice-1
        /* 033 */ 0x000, // índice-2
        /* 034 */ 0x000, // fib[i-1] temporal
    };

    // Inicializa memoria
    memset(mem, 0, sizeof(mem));
    memcpy(mem, example_program, sizeof(example_program));

    // Inicializa CPU
    pc = 0;
    acc = 0;
    x = 0;
    status.z = 0;
    status.h = 0;

    printf("\n=== INICIANDO EJECUCIÓN ===\n\n");
    
    // Ejecuta
    loop();

    printf("\n╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                    EJECUCIÓN COMPLETADA                      ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("ESTADO FINAL DE LA CPU:\n");
    printf("  AC (Acumulador):     0x%03X (%d decimal)\n", acc & 0xFFF, acc & 0xFFF);
    printf("  X  (Índice):         0x%03X (%d decimal)\n", x & 0xFFF, x & 0xFFF);
    printf("  PC (Contador):       0x%03X (%d decimal)\n", pc & 0xFFF, pc & 0xFFF);
    printf("\n");
    
    printf("SECUENCIA DE FIBONACCI GENERADA:\n");
    printf("┌────────┬─────────┬─────────┐\n");
    printf("│ Índice │   Hex   │ Decimal │\n");
    printf("├────────┼─────────┼─────────┤\n");
    for (int i = 0; i < 8; i++) {
        printf("│ fib[%d] │  0x%03X  │   %3d   │\n", i, mem[0x20 + i], mem[0x20 + i]);
    }
    printf("└────────┴─────────┴─────────┘\n");
    printf("\n");
    
    // Verificar la secuencia
    uint16_t expected[] = {0, 1, 1, 2, 3, 5, 8, 13};
    int correct = 1;
    for (int i = 0; i < 8; i++) {
        if (mem[0x20 + i] != expected[i]) {
            correct = 0;
            break;
        }
    }
    
    if (correct) {
        printf("✓ ÉXITO: La secuencia de Fibonacci es correcta!\n");
    } else {
        printf("✗ NOTA: La secuencia tiene diferencias con lo esperado\n");
    }
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - Secuencias recursivas se pueden implementar iterativamente\n");
    printf("  - Cada valor depende de los dos anteriores: fib[i] = fib[i-1] + fib[i-2]\n");
    printf("  - Uso de indexación para acceder a elementos previos del array\n");
    printf("  - Gestión de múltiples variables temporales\n");
    printf("\n");
    printf("MATEMÁTICAS:\n");
    printf("  La secuencia de Fibonacci aparece en la naturaleza:\n");
    printf("  - Patrones de crecimiento de plantas\n");
    printf("  - Espirales de caracoles\n");
    printf("  - Proporciones áureas\n");
    printf("\n");
    printf("OPTIMIZACIÓN:\n");
    printf("  Este algoritmo es O(n) en tiempo y espacio.\n");
    printf("  Solo necesitamos mantener los dos últimos valores,\n");
    printf("  pero aquí almacenamos toda la secuencia para demostración.\n");
    printf("\n");
    
    return 0;
}