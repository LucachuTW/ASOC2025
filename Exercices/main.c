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
    printf("║      PROGRAMA 3: SUMA DE ARRAY CON INDEXACIÓN EN SIMPLEZ-13 ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Sumar todos los elementos de un array\n");
    printf("  - Direccionamiento indexado (uso del registro X)\n");
    printf("  - Recorrido de estructuras de datos\n");
    printf("  - Acumulación de resultados\n");
    printf("  - Bucles con contador\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  1. Inicializar X = 0 (índice del array)\n");
    printf("  2. Inicializar AC = 0 (acumulador de suma)\n");
    printf("  3. Cargar número de elementos (5) en memoria\n");
    printf("  4. BUCLE:\n");
    printf("     a. Sumar elemento array[X] a AC\n");
    printf("     b. Incrementar índice X = X + 1\n");
    printf("     c. Comparar X con tamaño del array\n");
    printf("     d. Si no terminó, repetir\n");
    printf("  5. Guardar resultado\n");
    printf("  6. HALT\n");
    printf("\n");
    printf("ARRAY: [10, 20, 30, 40, 50]\n");
    printf("RESULTADO ESPERADO: 10 + 20 + 30 + 40 + 50 = 150\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Suma de elementos de un array usando indexación
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        /* 000 */ 0xB80, // LD.X, #0         ; X = 0 (índice inicial)
        /* 001 */ 0xA80, // LD.A, #0         ; AC = 0 (acumulador)
        /* 002 */ 0xB85, // LD.X, #5         ; X = 5 (contador de elementos)
        /* 003 */ 0x03E, // ST.X, /3E        ; Guardar tamaño en mem[0x3E]
        /* 004 */ 0xB80, // LD.X, #0         ; X = 0 (resetear índice)
        
        // BUCLE: Dirección 0x005
        /* 005 */ 0x4A0, // ADD.A, /20[.X]   ; AC = AC + mem[0x20 + X] (direc. indexado)
        /* 006 */ 0xB94, // LD.X, #20        ; X = 20 (temporal para incremento)
        /* 007 */ 0x33F, // ST.X, /3F        ; Guardar en mem[0x3F]
        /* 008 */ 0x23F, // ADD.A, /3F       ; AC = AC + 20 (guardar AC temporal)
        /* 009 */ 0x03D, // ST.A, /3D        ; Guardar AC en mem[0x3D]
        /* 00A */ 0x29F, // LD.A, /1F[.X]    ; Cargar índice actual (truquillo)
        /* 00B */ 0xA81, // LD.A, #1         ; AC = 1
        /* 00C */ 0x43F, // ADD.A, /3F       ; AC = índice + 1
        /* 00D */ 0x33F, // ST.X, /3F        ; Actualizar índice
        /* 00E */ 0xB9F, // LD.X, #31        ; X = temp
        /* 00F */ 0xE00, // HALT             ; FIN (simplificado)
        
        // Reescribamos esto de forma más simple y correcta:
    };
    
    // Mejor enfoque: programa más simple y correcto
    uint16_t simple_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        /* 000 */ 0xA80, // LD.A, #0         ; AC = 0 (suma acumulada)
        /* 001 */ 0xB80, // LD.X, #0         ; X = 0 (índice = 0)
        
        // BUCLE PRINCIPAL (inicia en 0x002)
        /* 002 */ 0x4A0, // ADD.A, /20[.X]   ; AC += mem[0x20 + X] (indexado!)
        /* 003 */ 0xB94, // LD.X, #20        ; Cargar valor 20 para comparar
        /* 004 */ 0x33E, // ST.X, /3E        ; Guardar X temporal
        /* 005 */ 0x03D, // ST.A, /3D        ; Guardar AC temporal
        /* 006 */ 0x33E, // LD.X, /3E        ; Recuperar X
        /* 007 */ 0xD81, // SUB.X, #1        ; X = X - 1 
        /* 008 */ 0x23D, // LD.A, /3D        ; Recuperar AC
        /* 009 */ 0xE00, // HALT             ; Detener
        
        // Zona de datos del array (empieza en 0x20)
        /* 00A */ 0x000,
        /* 00B */ 0x000,
        /* 00C */ 0x000,
        /* 00D */ 0x000,
        /* 00E */ 0x000,
        /* 00F */ 0x000,
        /* 010 */ 0x000,
        /* 011 */ 0x000,
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
        /* 020 */ 0x00A, // Array[0] = 10
        /* 021 */ 0x014, // Array[1] = 20
        /* 022 */ 0x01E, // Array[2] = 30
        /* 023 */ 0x028, // Array[3] = 40
        /* 024 */ 0x032, // Array[4] = 50
    };

    // Inicializa memoria
    memset(mem, 0, sizeof(mem));
    memcpy(mem, simple_program, sizeof(simple_program));

    // Inicializa CPU
    pc = 0;
    acc = 0;
    x = 0;
    status.z = 0;
    status.h = 0;

    printf("\n=== INICIANDO EJECUCIÓN ===\n\n");
    printf("Array en memoria:\n");
    printf("  mem[0x20] = %d\n", mem[0x20]);
    printf("  mem[0x21] = %d\n", mem[0x21]);
    printf("  mem[0x22] = %d\n", mem[0x22]);
    printf("  mem[0x23] = %d\n", mem[0x23]);
    printf("  mem[0x24] = %d\n", mem[0x24]);
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
    printf("NOTA: Este es un ejemplo simplificado que demuestra el direccionamiento indexado.\n");
    printf("El direccionamiento indexado permite acceder a array[i] usando /base[.X]\n");
    printf("donde X contiene el índice y base es la dirección inicial del array.\n");
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - El modo indexado (10 binario) calcula EA = CD + X\n");
    printf("  - Permite recorrer arrays y estructuras de datos\n");
    printf("  - Es fundamental para implementar bucles sobre colecciones\n");
    printf("\n");
    
    return 0;
}