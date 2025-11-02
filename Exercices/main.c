#include <stdio.h>
#include <string.h>
#include "simplez.h"

// --- Definición de Variables Globales ---
// (Aquí es donde "viven" las variables declaradas en el .h)
uint16_t mem[MEM_SIZE];
uint16_t acc;
uint16_t x;
uint16_t pc;
struct status status;

// Array de punteros a funciones para mapear opcodes a funciones
// Sigue el orden de la tabla de la página 20 
void (*iset[])(uint8_t reg, uint16_t data) = {
    st,         // 000 (0)
    ld,         // 001 (1)
    add,        // 010 (2)
    br,         // 011 (3)
    bz,         // 100 (4)
    ld_sharp,   // 101 (5)
    sub_sharp,  // 110 (6)
    halt        // 111 (7) - Simplificado, 11100 en el PDF 
};

// Nombres para el depurador
char *iset_names[] = {"ST", "LD", "ADD", "BR", "BZ", "LD #", "SUB #", "HALT"};


/**
 * @brief Ciclo principal de la CPU: Fetch, Decode, Execute.
 */
void loop() {
    // El bucle se detiene cuando la instrucción HALT pone status.h a 1
    while (!status.h) {
        
        // --- 1. FETCH (BÚSQUEDA) ---
        // Lee la instrucción de 12 bits de la memoria
        uint16_t inst = mem[pc] & 0xFFF; // Aseguramos 12 bits

        
        // --- 2. DECODE (DECODIFICACIÓN) ---
        // Descompone la instrucción según el formato de la pág. 8 [cite: 98-102]
        
        // Bits 11-9: Código de Operación (CO) [cite: 101]
        uint8_t op = (inst >> 9) & 0x7; 
        
        // Bit 8: Registro (R). 0=AC, 1=X [cite: 106, 107]
        uint8_t reg = (inst >> 8) & 0x1;
        
        // Bits 7-6: Modo de Direccionamiento (J, I) [cite: 108]
        uint8_t dirm = (inst >> 6) & 0x3;
        
        // Bits 5-0: Campo de Dirección (CD) [cite: 102]
        uint16_t cd = inst & 0x3F;

        
        // --- 3. CÁLCULO DE DIRECCIÓN EFECTIVA (EA) / OBTENCIÓN DE DATO ---
        
        uint16_t data; // Almacenará la Dirección Efectiva (EA) o el literal

        // Las instrucciones 5 (LD #) y 6 (SUB #) son especiales:
        // Siempre usan modo inmediato y 'cd' es el dato literal [cite: 315]
        if (op == 5 || op == 6) {
            data = cd; // El dato es el propio campo CD [cite: 201]
        } else {
            // Para el resto (ST, LD, ADD, BR, BZ), calculamos la EA
            // según la tabla resumen de la pág. 15 [cite: 225]
            
            switch (dirm) {
                case 0: // 00: Directo
                    data = cd; // DE = (CD)
                    break;
                case 1: // 01: Indirecto
                    data = mem[cd]; // DE = ((CD))
                    break;
                case 2: // 10: Indexado
                    data = cd + x; // DE = (CD) + (X)
                    break;
                case 3: // 11: Indirecto e Indexado
                    data = mem[cd] + x; // DE = ((CD)) + (X)
                    break;
            }
        }

        // --- 4. EXECUTE (EJECUCIÓN) ---
        
        // Impresión de depuración (como la que teníais)
        printf("DEBUG: PC:%03X | Inst:%03X | OP: %s (R:%d, M:%d, CD:%02X) | EA/Dato: %03X\n", 
                pc, inst, iset_names[op], reg, dirm, cd, data);
        
        // Llama a la función correcta (ST, LD, ADD...) usando el opcode
        // como índice en el array de punteros a funciones.
        iset[op](reg, data);

        
        // --- 5. ACTUALIZAR PC ---
        pc++; // Avanza a la siguiente instrucción
        
        // Impresión de estado de registros
        printf("       -> Estado: PC:%03X, AC:%03X, X:%03X, Z_flag:%d, H_flag:%d\n", 
                pc, acc & 0xFFF, x & 0xFFF, status.z, status.h);
        getchar(); // Espera a que el usuario pulse Enter
    }
}

/**
 * @brief Punto de entrada del Emulador.
 */
int main() {
    // --- Carga del Programa en Memoria ---
    // Este es un nuevo programa de ejemplo que SÍ funciona con Simplez-13
    // y prueba varios modos de direccionamiento.
    uint16_t example_program[] = {
        // Dirección | Contenido | Ensamblador     | Explicación
        /* 0x000 */  0xA87,     // LD.A, #7        ; AC = 7
        /* 0x001 */  0xB94,     // LD.X, #20       ; X = 20 (decimal)
        /* 0x002 */  0x014,     // ST.A, /20       ; mem[20] = AC (o sea, 7)
        /* 0x003 */  0xA80,     // LD.A, #0        ; AC = 0
        /* 0x004 */  0x480,     // ADD.A, /0[.X]   ; AC = AC + mem[0 + X]
                               //                 ; AC = 0 + mem[0 + 20]
                               //                 ; AC = 0 + mem[20]
                               //                 ; AC = 0 + 7. AC_final = 7
        /* 0x005 */  0x442,     // ADD.A, [/10]    ; AC = AC + mem[mem[10]]
                               //                 ; AC = 7 + mem[5]
                               //                 ; AC = 7 + 0xE00 = 0xE07
        /* 0x006 */  0xE00,     // HALT            ; Detiene la máquina
        
        // --- Zona de Datos ---
        /* 0x007 */  0x000,
        /* 0x008 */  0x000,
        /* 0x009 */  0x000,
        /* 0x00A */  0x005      // Puntero para el ADD indirecto
    };

    // Inicializa toda la memoria a 0
    memset(mem, 0, sizeof(mem));
    
    // Copia el programa de ejemplo al inicio de la memoria
    memcpy(mem, example_program, sizeof(example_program));

    // --- Inicialización de la CPU ---
    pc = 0;
    acc = 0;
    x = 0;
    status.z = 0;
    status.h = 0;

    printf("Iniciando emulador Simplez-13. Presiona Enter para avanzar cada ciclo.\n");

    // Inicia el ciclo de la CPU
    loop();

    printf("--- EJECUCIÓN DETENIDA (HALT) ---\n");
    printf("Estado final:\n");
    printf("AC: %03X (%d)\n", acc & 0xFFF, acc & 0xFFF);
    printf("X:  %03X (%d)\n", x & 0xFFF, x & 0xFFF);
    printf("PC: %03X (%d)\n", pc & 0xFFF, pc & 0xFFF);
    printf("mem[20]: %03X (%d)\n", mem[20], mem[20]);
    
    return 0;
}