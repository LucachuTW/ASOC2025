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
    printf("║   PROGRAMA 9: TODOS LOS MODOS DE DIRECCIONAMIENTO SIMPLEZ-13║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Demostrar todos los modos de direccionamiento disponibles\n");
    printf("\n");
    printf("MODOS DE DIRECCIONAMIENTO EN SIMPLEZ-13:\n");
    printf("┌──────┬─────────────────────────┬────────────────┬─────────────┐\n");
    printf("│ Bits │ Modo                    │ Notación       │ EA Cálculo  │\n");
    printf("├──────┼─────────────────────────┼────────────────┼─────────────┤\n");
    printf("│  00  │ Directo                 │ /dir           │ EA = CD     │\n");
    printf("│  01  │ Indirecto               │ [/dir]         │ EA = (CD)   │\n");
    printf("│  10  │ Indexado                │ /dir[.X]       │ EA = CD + X │\n");
    printf("│  11  │ Indirecto Indexado      │ [/dir][.X]     │ EA = (CD)+X │\n");
    printf("│  --  │ Inmediato (LD#, SUB#)   │ #valor         │ EA = CD     │\n");
    printf("└──────┴─────────────────────────┴────────────────┴─────────────┘\n");
    printf("\n");
    printf("LEYENDA:\n");
    printf("  CD  = Campo de Dirección (6 bits)\n");
    printf("  EA  = Effective Address (Dirección Efectiva)\n");
    printf("  (CD)= Contenido de la memoria en dirección CD\n");
    printf("  X   = Registro de Índice\n");
    printf("\n");
    printf("Este programa ejecutará una instrucción de cada modo.\n");
    printf("Observe cómo cambia el cálculo de EA en cada caso.\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Demostración de todos los modos
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        
        printf("\n>>> MODO INMEDIATO: LD #\n");
        /* 000 */ 0xA8F, // LD.A, #15        ; AC = 15 (inmediato)
                         //                  ; No usa EA, el dato es el propio CD
        
        printf("\n>>> MODO DIRECTO: ST /dir\n");
        /* 001 */ 0x020, // ST.A, /20        ; mem[0x20] = AC (directo)
                         //                  ; EA = CD = 0x20
        
        printf("\n>>> MODO INDIRECTO: LD [/dir]\n");
        /* 002 */ 0xB9E, // LD.X, #30        ; X = 0x30
        /* 003 */ 0x310, // ST.X, /10        ; mem[0x10] = 0x30 (crear puntero)
        /* 004 */ 0xB99, // LD.X, #25        ; X = 25
        /* 005 */ 0x330, // ST.X, /30        ; mem[0x30] = 25
        /* 006 */ 0x250, // LD.A, [/10]      ; AC = mem[mem[0x10]] (indirecto)
                         //                  ; EA = (0x10) = mem[0x10] = 0x30
                         //                  ; AC = mem[0x30] = 25
        
        printf("\n>>> MODO INDEXADO: ADD /dir[.X]\n");
        /* 007 */ 0xB83, // LD.X, #3         ; X = 3 (índice)
        /* 008 */ 0x4A0, // ADD.A, /20[.X]   ; AC = AC + mem[0x20 + X] (indexado)
                         //                  ; EA = CD + X = 0x20 + 3 = 0x23
                         //                  ; AC = AC + mem[0x23]
        
        printf("\n>>> MODO INDIRECTO INDEXADO: LD [/dir][.X]\n");
        /* 009 */ 0xA94, // LD.A, #20        ; AC = 0x20 (base del array)
        /* 00A */ 0x011, // ST.A, /11        ; mem[0x11] = 0x20 (puntero base)
        /* 00B */ 0xB82, // LD.X, #2         ; X = 2 (offset)
        /* 00C */ 0x2D1, // LD.A, [/11][.X]  ; AC = mem[mem[0x11] + X] (ind. indexado)
                         //                  ; EA = (0x11) + X = mem[0x11] + 2
                         //                  ; EA = 0x20 + 2 = 0x22
                         //                  ; AC = mem[0x22]
        
        printf("\n>>> MODO INMEDIATO: SUB #\n");
        /* 00D */ 0xC85, // SUB.A, #5        ; AC = AC - 5 (inmediato)
                         //                  ; No usa EA, el dato es el propio CD
        
        /* 00E */ 0x03F, // ST.A, /3F        ; Guardar resultado final
        /* 00F */ 0xE00, // HALT             ; Detener
        
        // Zona de datos y punteros
        /* 010 */ 0x000, // Puntero 1 (se llenará con 0x30)
        /* 011 */ 0x000, // Puntero 2 (se llenará con 0x20)
        /* 012 */ 0x000,
        /* 013 */ 0x000,
        /* 014 */ 0x000,
        /* 015 */ 0x000,
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
        
        // Array de ejemplo (0x20-0x24)
        /* 020 */ 0x00A, // array[0] = 10
        /* 021 */ 0x014, // array[1] = 20
        /* 022 */ 0x01E, // array[2] = 30
        /* 023 */ 0x028, // array[3] = 40
        /* 024 */ 0x032, // array[4] = 50
        
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
        /* 030 */ 0x000, // Dato indirecto (se llenará con 25)
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

    printf("\n=== CONFIGURACIÓN INICIAL ===\n");
    printf("Array en mem[0x20-0x24]: [10, 20, 30, 40, 50]\n");
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
    
    printf("VALORES RELEVANTES EN MEMORIA:\n");
    printf("  mem[0x10] = 0x%03X (%d) - Puntero 1\n", mem[0x10], mem[0x10]);
    printf("  mem[0x11] = 0x%03X (%d) - Puntero 2\n", mem[0x11], mem[0x11]);
    printf("  mem[0x20] = 0x%03X (%d) - Array[0]\n", mem[0x20], mem[0x20]);
    printf("  mem[0x22] = 0x%03X (%d) - Array[2]\n", mem[0x22], mem[0x22]);
    printf("  mem[0x23] = 0x%03X (%d) - Array[3]\n", mem[0x23], mem[0x23]);
    printf("  mem[0x30] = 0x%03X (%d) - Dato indirecto\n", mem[0x30], mem[0x30]);
    printf("  mem[0x3F] = 0x%03X (%d) - Resultado final\n", mem[0x3F], mem[0x3F]);
    printf("\n");
    
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                 RESUMEN DE MODOS UTILIZADOS                  ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("1. INMEDIATO (LD #15):\n");
    printf("   → AC = 15 directamente\n");
    printf("   → Uso: Constantes, valores literales\n");
    printf("\n");
    printf("2. DIRECTO (ST /20):\n");
    printf("   → mem[0x20] = AC\n");
    printf("   → Uso: Variables simples en direcciones fijas\n");
    printf("\n");
    printf("3. INDIRECTO (LD [/10]):\n");
    printf("   → AC = mem[mem[0x10]]\n");
    printf("   → Uso: Punteros, acceso indirecto\n");
    printf("\n");
    printf("4. INDEXADO (ADD /20[.X]):\n");
    printf("   → AC += mem[0x20 + X]\n");
    printf("   → Uso: Arrays, estructuras secuenciales\n");
    printf("\n");
    printf("5. INDIRECTO INDEXADO (LD [/11][.X]):\n");
    printf("   → AC = mem[mem[0x11] + X]\n");
    printf("   → Uso: Arrays de punteros, estructuras complejas\n");
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  Los modos de direccionamiento son fundamentales en arquitectura de computadoras.\n");
    printf("  Cada modo tiene un propósito específico y permite diferentes patrones de acceso.\n");
    printf("  La elección del modo correcto optimiza tanto el código como el rendimiento.\n");
    printf("\n");
    printf("COMPARACIÓN CON C:\n");
    printf("  Inmediato:          int a = 5;\n");
    printf("  Directo:            variable = valor;\n");
    printf("  Indirecto:          *ptr = valor;\n");
    printf("  Indexado:           array[i] = valor;\n");
    printf("  Ind. Indexado:      ptr[i] = valor;\n");
    printf("\n");
    printf("APLICACIONES:\n");
    printf("  - Gestión de memoria dinámica\n");
    printf("  - Implementación de estructuras de datos\n");
    printf("  - Optimización de código\n");
    printf("  - Sistemas operativos y compiladores\n");
    printf("\n");
    
    return 0;
}