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
    printf("║      PROGRAMA 7: COPIA DE BLOQUES DE MEMORIA SIMPLEZ-13     ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Copiar un bloque de memoria de una zona a otra\n");
    printf("  - Operaciones de memoria en bloque\n");
    printf("  - Direccionamiento indexado para origen y destino\n");
    printf("  - Manipulación de múltiples índices\n");
    printf("  - Transferencia de datos\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  origen = 0x20\n");
    printf("  destino = 0x30\n");
    printf("  tamaño = 5\n");
    printf("  para i = 0 hasta tamaño-1:\n");
    printf("      destino[i] = origen[i]\n");
    printf("\n");
    printf("DATOS ORIGEN: [11, 22, 33, 44, 55] en 0x20-0x24\n");
    printf("DESTINO: 0x30-0x34 (inicialmente vacío)\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Copiar bloque de memoria
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        
        // Inicialización
        /* 000 */ 0xB80, // LD.X, #0         ; X = 0 (índice/contador)
        
        // BUCLE DE COPIA
        /* 001 */ 0xB94, // LD.X, #20        ; Cargar 20 para comparar
        /* 002 */ 0x03F, // ST.X, /3F        ; Guardar temp
        /* 003 */ 0x23F, // LD.A, /3F        ; AC = 20
        /* 004 */ 0xB80, // LD.X, #0         ; X = 0 (resetear índice)
        
        // Verificar si terminamos (índice < 5)
        /* 005 */ 0x33E, // ST.X, /3E        ; Guardar índice actual
        /* 006 */ 0x33E, // LD.X, /3E        ; Cargar índice
        /* 007 */ 0xD85, // SUB.X, #5        ; X = X - 5
        /* 008 */ 0x800, // BZ, /00          ; Si X == 0, terminamos
        
        // Restaurar índice y copiar
        /* 009 */ 0x33E, // LD.X, /3E        ; X = índice actual
        /* 00A */ 0x2A0, // LD.A, /20[.X]    ; AC = mem[0x20 + X] (origen)
        /* 00B */ 0x0B0, // ST.A, /30[.X]    ; mem[0x30 + X] = AC (destino)
        
        // Incrementar índice
        /* 00C */ 0x33E, // LD.X, /3E        ; Cargar índice
        /* 00D */ 0xB81, // LD.X, #1         ; X = 1
        /* 00E */ 0x23E, // LD.A, /3E        ; AC = índice
        /* 00F */ 0xA81, // LD.A, #1         ; AC = 1
        /* 010 */ 0x43E, // ADD.A, /3E       ; AC = índice + 1
        /* 011 */ 0x03E, // ST.A, /3E        ; Guardar nuevo índice
        /* 012 */ 0x605, // BR, /05          ; Repetir bucle
        
        /* 013 */ 0xE00, // HALT             ; Detener
        
        // Zona de variables
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
        
        // BLOQUE ORIGEN (0x20-0x24)
        /* 020 */ 0x00B, // 11
        /* 021 */ 0x016, // 22
        /* 022 */ 0x021, // 33
        /* 023 */ 0x02C, // 44
        /* 024 */ 0x037, // 55
        
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
        
        // BLOQUE DESTINO (0x30-0x34) - inicialmente vacío
        /* 030 */ 0x000,
        /* 031 */ 0x000,
        /* 032 */ 0x000,
        /* 033 */ 0x000,
        /* 034 */ 0x000,
        
        /* 035 */ 0x000,
        /* 036 */ 0x000,
        /* 037 */ 0x000,
        /* 038 */ 0x000,
        /* 039 */ 0x000,
        /* 03A */ 0x000,
        /* 03B */ 0x000,
        /* 03C */ 0x000,
        /* 03D */ 0x000,
        /* 03E */ 0x000, // índice
        /* 03F */ 0x000, // temp
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

    printf("\n=== ESTADO INICIAL ===\n\n");
    printf("BLOQUE ORIGEN (0x20-0x24):\n");
    for (int i = 0; i < 5; i++) {
        printf("  mem[0x%02X] = %d\n", 0x20 + i, mem[0x20 + i]);
    }
    printf("\nBLOQUE DESTINO (0x30-0x34) - antes de copiar:\n");
    for (int i = 0; i < 5; i++) {
        printf("  mem[0x%02X] = %d\n", 0x30 + i, mem[0x30 + i]);
    }
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
    
    printf("BLOQUE ORIGEN (0x20-0x24):\n");
    for (int i = 0; i < 5; i++) {
        printf("  mem[0x%02X] = %d\n", 0x20 + i, mem[0x20 + i]);
    }
    
    printf("\nBLOQUE DESTINO (0x30-0x34) - después de copiar:\n");
    for (int i = 0; i < 5; i++) {
        printf("  mem[0x%02X] = %d\n", 0x30 + i, mem[0x30 + i]);
    }
    printf("\n");
    
    // Verificar si la copia fue exitosa
    int success = 1;
    for (int i = 0; i < 5; i++) {
        if (mem[0x30 + i] != mem[0x20 + i]) {
            success = 0;
            break;
        }
    }
    
    if (success) {
        printf("✓ ÉXITO: El bloque fue copiado correctamente!\n");
    } else {
        printf("✗ NOTA: Hay diferencias entre origen y destino\n");
    }
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - Copia de bloques requiere direccionamiento indexado en origen y destino\n");
    printf("  - Se usa el mismo índice para acceder a ambas zonas de memoria\n");
    printf("  - Operación fundamental para manipulación de datos\n");
    printf("  - Similar a memcpy() en C\n");
    printf("\n");
    printf("APLICACIÓN:\n");
    printf("  Esta técnica es esencial para:\n");
    printf("  - Mover estructuras de datos\n");
    printf("  - Implementar pilas y colas\n");
    printf("  - Gestión de buffers\n");
    printf("  - Operaciones de strings\n");
    printf("\n");
    
    return 0;
}