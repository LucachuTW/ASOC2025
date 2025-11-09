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
    printf("║       PROGRAMA 2: CONTADOR ASCENDENTE EN SIMPLEZ-13         ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Implementar un bucle contador de 1 a 10\n");
    printf("  - Uso de bucles con BZ (Branch if Zero)\n");
    printf("  - Operación de resta inmediata (SUB #)\n");
    printf("  - Control de flujo condicional\n");
    printf("  - Actualización del flag Z\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  1. Inicializar contador en X = 10\n");
    printf("  2. Inicializar suma en AC = 0\n");
    printf("  3. BUCLE:\n");
    printf("     a. Sumar contador actual a AC\n");
    printf("     b. Restar 1 al contador (X = X - 1)\n");
    printf("     c. Si X != 0, volver al paso 3\n");
    printf("  4. Guardar resultado\n");
    printf("  5. HALT\n");
    printf("\n");
    printf("RESULTADO ESPERADO: AC = 1+2+3+4+5+6+7+8+9+10 = 55\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Contador que suma de 1 a 10
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        /* 000 */ 0xB8A, // LD.X, #10        ; X = 10 (contador)
        /* 001 */ 0xA80, // LD.A, #0         ; AC = 0 (acumulador de suma)
        /* 002 */ 0x03E, // ST.X, /3E        ; Guardar X en mem[0x3E] (temporal)
        /* 003 */ 0x23E, // ADD.A, /3E       ; AC = AC + X
        /* 004 */ 0xD81, // SUB.X, #1        ; X = X - 1
        /* 005 */ 0x803, // BZ.X, /03        ; Si X == 0, salta a 0x03 (siguiente instr después del bucle)
        /* 006 */ 0x602, // BR, /02          ; Salto incondicional a 0x02 (repetir bucle)
        /* 007 */ 0x03F, // ST.A, /3F        ; Guardar resultado en mem[0x3F]
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
    printf("  mem[0x3E] = 0x%03X (%d) - Última iteración del contador\n", mem[0x3E], mem[0x3E]);
    printf("  mem[0x3F] = 0x%03X (%d) - RESULTADO: Suma de 1 a 10\n", mem[0x3F], mem[0x3F]);
    printf("\n");
    printf("CÁLCULO: 1+2+3+4+5+6+7+8+9+10 = 55\n");
    printf("\n");
    
    if (mem[0x3F] == 55) {
        printf("✓ ÉXITO: El resultado es correcto!\n");
    } else {
        printf("✗ ERROR: Se esperaba 55, se obtuvo %d\n", mem[0x3F]);
    }
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - Los bucles se implementan con saltos condicionales (BZ) e incondicionales (BR)\n");
    printf("  - El flag Z se actualiza automáticamente después de operaciones aritméticas\n");
    printf("  - SUB # permite decrementar contadores eficientemente\n");
    printf("\n");
    
    return 0;
}