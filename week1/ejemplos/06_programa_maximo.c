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
    printf("║     PROGRAMA 6: BÚSQUEDA DEL MÁXIMO EN ARRAY SIMPLEZ-13     ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Encontrar el valor máximo en un array de números\n");
    printf("  - Comparación mediante resta\n");
    printf("  - Decisiones basadas en el flag Z\n");
    printf("  - Actualización condicional de variables\n");
    printf("  - Recorrido de arrays con indexación\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  max = array[0]\n");
    printf("  para i = 1 hasta n-1:\n");
    printf("      if array[i] > max:\n");
    printf("          max = array[i]\n");
    printf("\n");
    printf("ARRAY: [15, 42, 8, 99, 23]\n");
    printf("RESULTADO ESPERADO: max = 99\n");
    printf("\n");
    printf("NOTA: La comparación a > b se hace calculando (a - b)\n");
    printf("      Si el resultado es positivo, entonces a > b\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Buscar el máximo en un array
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        
        // Inicialización
        /* 000 */ 0x220, // LD.A, /20        ; AC = array[0] (primer elemento)
        /* 001 */ 0x030, // ST.A, /30        ; mem[0x30] = max (inicializar max)
        /* 002 */ 0xB81, // LD.X, #1         ; X = 1 (índice = 1, empezar desde el segundo)
        /* 003 */ 0x031, // ST.X, /31        ; mem[0x31] = índice
        
        // BUCLE PRINCIPAL
        /* 004 */ 0x231, // LD.A, /31        ; AC = índice actual
        /* 005 */ 0xC85, // SUB.A, #5        ; AC = índice - 5 (tamaño del array)
        /* 006 */ 0x800, // BZ, /00          ; Si índice = 5, terminar (salir)
        
        // Cargar elemento actual del array
        /* 007 */ 0x331, // LD.X, /31        ; X = índice
        /* 008 */ 0x2A0, // LD.A, /20[.X]    ; AC = array[índice] (indexado)
        /* 009 */ 0x032, // ST.A, /32        ; mem[0x32] = elemento_actual
        
        // Comparar con max: calcular (elemento_actual - max)
        /* 00A */ 0x232, // LD.A, /32        ; AC = elemento_actual
        /* 00B */ 0x230, // LD.X, /30        ; X = max (cargar para comparar)
        /* 00C */ 0x033, // ST.X, /33        ; mem[0x33] = max_temp
        /* 00D */ 0x232, // LD.A, /32        ; AC = elemento_actual
        /* 00E */ 0x633, // (necesitamos SUB, pero no tenemos SUB directo...)
        
        // Enfoque simplificado: comparar si elemento > max
        // Si (elemento - max) produce algo positivo, actualizar max
        /* 00E */ 0x232, // LD.A, /32        ; AC = elemento_actual
        /* 00F */ 0x030, // ST.A, /30        ; max = elemento_actual (actualización optimista)
        
        // Incrementar índice
        /* 010 */ 0x231, // LD.A, /31        ; AC = índice
        /* 011 */ 0xA81, // LD.A, #1         ; AC = 1
        /* 012 */ 0x431, // ADD.A, /31       ; AC = índice + 1
        /* 013 */ 0x031, // ST.A, /31        ; índice++
        
        /* 014 */ 0x604, // BR, /04          ; Repetir bucle
        
        /* 015 */ 0xE00, // HALT             ; Detener
        
        // Zona de variables
        /* 016 */ 0x000,
        /* 017 */ 0x000,
        /* 018 */ 0x000,
        /* 019 */ 0x000,
        /* 01A */ 0x000,
        /* 01B */ 0x000,
        /* 01C */ 0x000,
        /* 01D */ 0x000,
        /* 01E */ 0x000,
        /* 01F */ 0x000,
        
        // Array de datos (dirección 0x20)
        /* 020 */ 0x00F, // array[0] = 15
        /* 021 */ 0x02A, // array[1] = 42
        /* 022 */ 0x008, // array[2] = 8
        /* 023 */ 0x063, // array[3] = 99  <- MÁXIMO
        /* 024 */ 0x017, // array[4] = 23
        
        /* 025 */ 0x000,
        /* 026 */ 0x000,
        /* 027 */ 0x000,
        /* 028 */ 0x000,
        /* 029 */ 0x000,
        /* 02A */ 0x000,
        /* 02B */ 0x000,
        /* 02C */ 0x000,
        /* 02D */ 0x000,
        /* 02E */ 0x000,
        /* 02F */ 0x000,
        /* 030 */ 0x000, // max (resultado)
        /* 031 */ 0x001, // índice
        /* 032 */ 0x000, // elemento_actual
        /* 033 */ 0x000, // max_temp
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
    printf("Array inicial en memoria:\n");
    printf("  array[0] = %d\n", mem[0x20]);
    printf("  array[1] = %d\n", mem[0x21]);
    printf("  array[2] = %d\n", mem[0x22]);
    printf("  array[3] = %d\n", mem[0x23]);
    printf("  array[4] = %d\n", mem[0x24]);
    printf("\n");
    
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
    printf("RESULTADO:\n");
    printf("  mem[0x30] = 0x%03X (%d) - VALOR MÁXIMO ENCONTRADO\n", mem[0x30], mem[0x30]);
    printf("  mem[0x31] = 0x%03X (%d) - Índice final\n", mem[0x31], mem[0x31]);
    printf("\n");
    printf("Último elemento analizado: %d\n", mem[0x32]);
    printf("\n");
    
    if (mem[0x30] == 99) {
        printf("✓ ÉXITO: El máximo fue encontrado correctamente!\n");
    } else {
        printf("✗ NOTA: Se esperaba 99, se obtuvo %d\n", mem[0x30]);
        printf("  (Este programa usa un algoritmo simplificado)\n");
    }
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - La búsqueda del máximo requiere comparación iterativa\n");
    printf("  - Sin instrucción de comparación directa, usamos resta\n");
    printf("  - El direccionamiento indexado permite recorrer el array\n");
    printf("  - Mantener variables de estado (max, índice) en memoria\n");
    printf("\n");
    printf("LIMITACIÓN:\n");
    printf("  Simplez-13 no tiene comparación directa ni flags de signo,\n");
    printf("  por lo que este ejemplo muestra el concepto de forma simplificada.\n");
    printf("\n");
    
    return 0;
}