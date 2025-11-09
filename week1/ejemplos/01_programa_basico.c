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
    printf("║      PROGRAMA 1: OPERACIONES BÁSICAS EN SIMPLEZ-13          ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Demostrar las operaciones fundamentales\n");
    printf("  - Carga inmediata (LD #)\n");
    printf("  - Almacenamiento en memoria (ST)\n");
    printf("  - Suma de valores (ADD)\n");
    printf("  - Modo de direccionamiento directo\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  1. Cargar valor 5 en AC\n");
    printf("  2. Almacenar AC en dirección 0x20\n");
    printf("  3. Cargar valor 3 en AC\n");
    printf("  4. Almacenar AC en dirección 0x21\n");
    printf("  5. Cargar valor 0 en AC\n");
    printf("  6. Sumar contenido de 0x20 a AC (AC = 0 + 5 = 5)\n");
    printf("  7. Sumar contenido de 0x21 a AC (AC = 5 + 3 = 8)\n");
    printf("  8. Almacenar resultado en 0x22\n");
    printf("  9. HALT\n");
    printf("\n");
    printf("RESULTADO ESPERADO: mem[0x22] = 8\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Suma simple de dos números
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        /* 000 */ 0xA85, // LD.A, #5         ; AC = 5
        /* 001 */ 0x020, // ST.A, /20        ; mem[0x20] = AC = 5
        /* 002 */ 0xA83, // LD.A, #3         ; AC = 3
        /* 003 */ 0x021, // ST.A, /21        ; mem[0x21] = AC = 3
        /* 004 */ 0xA80, // LD.A, #0         ; AC = 0 (resetear)
        /* 005 */ 0x420, // ADD.A, /20       ; AC = AC + mem[0x20] = 0 + 5 = 5
        /* 006 */ 0x421, // ADD.A, /21       ; AC = AC + mem[0x21] = 5 + 3 = 8
        /* 007 */ 0x022, // ST.A, /22        ; mem[0x22] = AC = 8 (guardar resultado)
        /* 008 */ 0xE00, // HALT             ; Detener
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
    printf("VALORES EN MEMORIA:\n");
    printf("  mem[0x20] = 0x%03X (%d) - Primer operando\n", mem[0x20], mem[0x20]);
    printf("  mem[0x21] = 0x%03X (%d) - Segundo operando\n", mem[0x21], mem[0x21]);
    printf("  mem[0x22] = 0x%03X (%d) - RESULTADO: 5 + 3 = 8\n", mem[0x22], mem[0x22]);
    printf("\n");
    
    if (mem[0x22] == 8) {
        printf("✓ ÉXITO: El resultado es correcto!\n");
    } else {
        printf("✗ ERROR: Se esperaba 8, se obtuvo %d\n", mem[0x22]);
    }
    printf("\n");
    
    return 0;
}