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
    printf("║   PROGRAMA 4: DIRECCIONAMIENTO INDIRECTO (PUNTEROS) SIMPLEZ-13║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Demostrar el uso de punteros (direccionamiento indirecto)\n");
    printf("  - Direccionamiento indirecto simple [/dir]\n");
    printf("  - Direccionamiento indirecto indexado [/dir][.X]\n");
    printf("  - Concepto de punteros en bajo nivel\n");
    printf("  - Acceso indirecto a datos\n");
    printf("\n");
    printf("MODOS DE DIRECCIONAMIENTO:\n");
    printf("  - Directo (00):              EA = CD\n");
    printf("  - Indirecto (01):            EA = mem[CD]  (como *ptr)\n");
    printf("  - Indexado (10):             EA = CD + X\n");
    printf("  - Indirecto Indexado (11):   EA = mem[CD] + X  (como ptr[i])\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  1. Crear 'puntero' en mem[0x10] que apunta a mem[0x30]\n");
    printf("  2. Almacenar valor 42 en mem[0x30]\n");
    printf("  3. Leer valor usando direccionamiento indirecto\n");
    printf("  4. Crear segundo puntero con offset\n");
    printf("  5. Usar direccionamiento indirecto indexado\n");
    printf("\n");
    printf("ANALOGÍA CON C:\n");
    printf("  int valor = 42;\n");
    printf("  int *ptr = &valor;    // ptr apunta a valor\n");
    printf("  int x = *ptr;         // x = 42 (indirección)\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Demostración de direccionamiento indirecto
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        
        // PARTE 1: Direccionamiento Indirecto Simple
        /* 000 */ 0xA9E, // LD.A, #30        ; AC = 0x30 (dirección destino)
        /* 001 */ 0x010, // ST.A, /10        ; mem[0x10] = 0x30 (crear puntero)
        /* 002 */ 0xB9A, // LD.X, #26        ; X = 26 (valor 42 en decimal)
        /* 003 */ 0x330, // ST.X, /30        ; mem[0x30] = 42 (guardar valor)
        
        /* 004 */ 0xA80, // LD.A, #0         ; AC = 0 (limpiar)
        /* 005 */ 0x250, // LD.A, [/10]      ; AC = mem[mem[0x10]]
                         //                  ; AC = mem[0x30] = 42
                         //                  ; ¡Indirección! Como *ptr en C
        
        /* 006 */ 0x03A, // ST.A, /3A        ; mem[0x3A] = AC (guardar resultado)
        
        // PARTE 2: Direccionamiento Indirecto Indexado
        /* 007 */ 0xA94, // LD.A, #20        ; AC = 0x20 (base del array)
        /* 008 */ 0x011, // ST.A, /11        ; mem[0x11] = 0x20 (puntero base)
        
        /* 009 */ 0xB83, // LD.X, #3         ; X = 3 (offset/índice)
        /* 00A */ 0x2D1, // LD.A, [/11][.X]  ; AC = mem[mem[0x11] + X]
                         //                  ; AC = mem[0x20 + 3]
                         //                  ; AC = mem[0x23] = 77
                         //                  ; ¡Como ptr[i] en C!
        
        /* 00B */ 0x03B, // ST.A, /3B        ; mem[0x3B] = AC (guardar)
        
        /* 00C */ 0xE00, // HALT             ; Detener
        
        // Zona de datos auxiliares
        /* 00D */ 0x000,
        /* 00E */ 0x000,
        /* 00F */ 0x000,
        /* 010 */ 0x000, // Aquí se guardará el puntero 1
        /* 011 */ 0x000, // Aquí se guardará el puntero 2
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
        /* 022 */ 0x032, // Array[2] = 50
        /* 023 */ 0x04D, // Array[3] = 77  <- Este será accedido indirectamente
        /* 024 */ 0x064, // Array[4] = 100
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
        /* 030 */ 0x000, // Aquí se guardará el valor 42
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
    printf("Configuración inicial:\n");
    printf("  Array en mem[0x20-0x24]: [10, 20, 50, 77, 100]\n");
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
    printf("PUNTEROS Y VALORES:\n");
    printf("  mem[0x10] = 0x%03X (%d) - Puntero 1 (apunta a 0x30)\n", mem[0x10], mem[0x10]);
    printf("  mem[0x11] = 0x%03X (%d) - Puntero 2 (apunta a 0x20)\n", mem[0x11], mem[0x11]);
    printf("  mem[0x30] = 0x%03X (%d) - Valor apuntado por puntero 1\n", mem[0x30], mem[0x30]);
    printf("\n");
    printf("RESULTADOS:\n");
    printf("  mem[0x3A] = 0x%03X (%d) - Valor leído con indirección simple\n", mem[0x3A], mem[0x3A]);
    printf("  mem[0x3B] = 0x%03X (%d) - Valor leído con indirección indexada\n", mem[0x3B], mem[0x3B]);
    printf("\n");
    
    if (mem[0x3A] == 42 && mem[0x3B] == 77) {
        printf("✓ ÉXITO: Ambos accesos indirectos funcionaron correctamente!\n");
    } else {
        printf("✗ ERROR en los accesos indirectos\n");
    }
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - Indirecto (01):         mem[CD] es como *ptr\n");
    printf("  - Indirecto Indexado (11): mem[CD]+X es como ptr[i]\n");
    printf("  - Los punteros son simplemente direcciones almacenadas en memoria\n");
    printf("  - El direccionamiento indirecto permite estructuras de datos dinámicas\n");
    printf("\n");
    printf("COMPARACIÓN CON C:\n");
    printf("  Simplez-13:    LD.A, [/10]     →  C: a = *ptr;\n");
    printf("  Simplez-13:    LD.A, [/11][.X] →  C: a = ptr[x];\n");
    printf("\n");
    
    return 0;
}