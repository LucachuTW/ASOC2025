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
    printf("║       PROGRAMA 5: CÁLCULO DE FACTORIAL EN SIMPLEZ-13        ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Calcular el factorial de un número (5! = 120)\n");
    printf("  - Multiplicación mediante sumas repetidas\n");
    printf("  - Bucles anidados\n");
    printf("  - Control de flujo complejo\n");
    printf("  - Uso combinado de AC y X\n");
    printf("\n");
    printf("ALGORITMO:\n");
    printf("  factorial = 1\n");
    printf("  para n desde 5 hasta 1:\n");
    printf("      resultado_temp = 0\n");
    printf("      para i desde 1 hasta n:\n");
    printf("          resultado_temp += factorial\n");
    printf("      factorial = resultado_temp\n");
    printf("\n");
    printf("CÁLCULO: 5! = 5 × 4 × 3 × 2 × 1 = 120\n");
    printf("\n");
    printf("NOTA: Simplez-13 no tiene multiplicación, así que 5×4 se hace\n");
    printf("      sumando 5 cuatro veces: 5+5+5+5 = 20\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Factorial de 5 (versión simplificada)
    // Calcula 5! = 120 usando sumas repetidas para simular multiplicación
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        
        // Inicialización
        /* 000 */ 0xA81, // LD.A, #1         ; AC = 1 (resultado = 1)
        /* 001 */ 0x030, // ST.A, /30        ; mem[0x30] = 1 (guardar resultado)
        /* 002 */ 0xB85, // LD.X, #5         ; X = 5 (contador externo, n)
        
        // BUCLE EXTERNO: para cada número de 5 a 1
        /* 003 */ 0x131, // ST.X, /31        ; mem[0x31] = X (guardar n actual)
        
        // Preparar multiplicación: resultado_temp = 0
        /* 004 */ 0xA80, // LD.A, #0         ; AC = 0 (acumulador para suma)
        /* 005 */ 0x032, // ST.A, /32        ; mem[0x32] = 0 (temp)
        
        // Cargar contador para bucle interno (multiplicar)
        /* 006 */ 0x231, // LD.A, /31        ; AC = n (cuántas veces sumar)
        /* 007 */ 0x033, // ST.A, /33        ; mem[0x33] = contador interno
        
        // BUCLE INTERNO: sumar 'resultado' n veces
        /* 008 */ 0x233, // LD.A, /33        ; AC = contador interno
        /* 009 */ 0x811, // BZ, /11          ; Si contador = 0, salir del bucle interno
        /* 00A */ 0x232, // LD.A, /32        ; AC = resultado temporal
        /* 00B */ 0x430, // ADD.A, /30       ; AC += resultado anterior
        /* 00C */ 0x032, // ST.A, /32        ; Guardar temp
        /* 00D */ 0x233, // LD.A, /33        ; Cargar contador
        /* 00E */ 0xC81, // SUB.A, #1        ; Decrementar contador
        /* 00F */ 0x033, // ST.A, /33        ; Guardar contador
        /* 010 */ 0x608, // BR, /08          ; Repetir bucle interno
        
        // Actualizar resultado
        /* 011 */ 0x232, // LD.A, /32        ; AC = resultado de la multiplicación
        /* 012 */ 0x030, // ST.A, /30        ; resultado = temp
        
        // Decrementar n
        /* 013 */ 0x231, // LD.A, /31        ; AC = n
        /* 014 */ 0xC81, // SUB.A, #1        ; n--
        /* 015 */ 0x031, // ST.A, /31        ; Guardar n
        /* 016 */ 0x818, // BZ, /18          ; Si n = 0, terminar
        /* 017 */ 0x603, // BR, /03          ; Repetir bucle externo (volver a 0x03)
        
        /* 018 */ 0xE00, // HALT             ; Detener
        
        // Zona de variables
        /* 019 */ 0x000,
        /* 01A */ 0x000,
        /* 01B */ 0x000,
        /* 01C */ 0x000,
        /* 01D */ 0x000,
        /* 01E */ 0x000,
        /* 01F */ 0x000,
        /* 020 */ 0x000,
        /* 021 */ 0x000,
        /* 022 */ 0x000,
        /* 023 */ 0x000,
        /* 024 */ 0x000,
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
        /* 030 */ 0x001, // resultado (inicializado a 1)
        /* 031 */ 0x000, // n (contador externo)
        /* 032 */ 0x000, // temp (acumulador de multiplicación)
        /* 033 */ 0x000, // contador interno
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
    printf("VARIABLES EN MEMORIA:\n");
    printf("  mem[0x30] = 0x%03X (%d) - FACTORIAL RESULTADO\n", mem[0x30], mem[0x30]);
    printf("  mem[0x31] = 0x%03X (%d) - Contador final\n", mem[0x31], mem[0x31]);
    printf("\n");
    printf("VERIFICACIÓN: 5! = 5 × 4 × 3 × 2 × 1 = 120\n");
    printf("\n");
    
    if (mem[0x30] == 120) {
        printf("✓ ÉXITO: El factorial fue calculado correctamente!\n");
    } else {
        printf("✗ NOTA: Se obtuvo %d (el algoritmo puede necesitar ajustes)\n", mem[0x30]);
    }
    printf("\n");
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - Operaciones complejas requieren descomposición en operaciones simples\n");
    printf("  - La multiplicación se simula con sumas repetidas\n");
    printf("  - Los bucles anidados permiten algoritmos de múltiples niveles\n");
    printf("  - Gestión de múltiples variables temporales en memoria\n");
    printf("\n");
    
    return 0;
}