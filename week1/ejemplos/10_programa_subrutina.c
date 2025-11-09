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
    printf("║   PROGRAMA 10: SIMULACIÓN DE SUBRUTINAS EN SIMPLEZ-13       ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("OBJETIVO: Simular llamadas a subrutinas y retornos\n");
    printf("  - Saltos incondicionales con BR\n");
    printf("  - Simulación de stack para direcciones de retorno\n");
    printf("  - Retorno de subrutinas\n");
    printf("  - Estructura modular del código\n");
    printf("\n");
    printf("CONCEPTO:\n");
    printf("  Las subrutinas (funciones) son bloques de código reutilizables.\n");
    printf("  Para implementarlas en Simplez-13 necesitamos:\n");
    printf("    1. Guardar la dirección de retorno antes del salto\n");
    printf("    2. Saltar a la subrutina (BR)\n");
    printf("    3. Ejecutar el código de la subrutina\n");
    printf("    4. Recuperar la dirección de retorno\n");
    printf("    5. Saltar de vuelta (BR indirecto)\n");
    printf("\n");
    printf("PROGRAMA:\n");
    printf("  main():\n");
    printf("    - Inicializar valores\n");
    printf("    - Llamar a subrutina_suma()\n");
    printf("    - Usar resultado\n");
    printf("    - Llamar a subrutina_doble()\n");
    printf("    - HALT\n");
    printf("\n");
    printf("  subrutina_suma():  suma dos valores\n");
    printf("  subrutina_doble(): duplica un valor\n");
    printf("\n");
    printf("Presiona Enter para comenzar...\n");
    getchar();

    // Programa: Simulación de subrutinas
    uint16_t example_program[] = {
        // Dir  | Hex   | Ensamblador      | Explicación
        
        // ============ PROGRAMA PRINCIPAL (MAIN) ============
        /* 000 */ 0xA85, // LD.A, #5         ; AC = 5 (primer operando)
        /* 001 */ 0x030, // ST.A, /30        ; mem[0x30] = 5
        /* 002 */ 0xA83, // LD.A, #3         ; AC = 3 (segundo operando)
        /* 003 */ 0x031, // ST.A, /31        ; mem[0x31] = 3
        
        // Llamada a subrutina_suma: guardar dirección de retorno
        /* 004 */ 0xA88, // LD.A, #8         ; AC = 8 (dirección siguiente a BR)
        /* 005 */ 0x032, // ST.A, /32        ; mem[0x32] = dirección_retorno
        /* 006 */ 0x610, // BR, /10          ; Saltar a subrutina_suma (0x10)
        
        // RETORNO DE subrutina_suma (el resultado está en AC)
        /* 007 */ 0x033, // ST.A, /33        ; mem[0x33] = resultado_suma
        
        // Llamada a subrutina_doble
        /* 008 */ 0xA8C, // LD.A, #12        ; AC = 12 (dirección de retorno)
        /* 009 */ 0x032, // ST.A, /32        ; mem[0x32] = dirección_retorno
        /* 00A */ 0x233, // LD.A, /33        ; AC = resultado anterior
        /* 00B */ 0x618, // BR, /18          ; Saltar a subrutina_doble (0x18)
        
        // RETORNO DE subrutina_doble
        /* 00C */ 0x034, // ST.A, /34        ; mem[0x34] = resultado_final
        /* 00D */ 0xE00, // HALT             ; FIN DEL PROGRAMA
        
        /* 00E */ 0x000, // (espacio)
        /* 00F */ 0x000, // (espacio)
        
        // ============ SUBRUTINA: SUMA ============
        // Suma los valores en mem[0x30] y mem[0x31]
        // Retorna el resultado en AC
        /* 010 */ 0x230, // LD.A, /30        ; AC = mem[0x30]
        /* 011 */ 0x431, // ADD.A, /31       ; AC = AC + mem[0x31]
        /* 012 */ 0x035, // ST.A, /35        ; Guardar resultado temporalmente
        
        // Retornar: saltar a la dirección guardada en mem[0x32]
        /* 013 */ 0x235, // LD.A, /35        ; Recuperar resultado
        /* 014 */ 0x632, // BR, [/32]        ; Salto indirecto a dirección_retorno
                         //                  ; BR usa modo 01 (indirecto)
                         //                  ; Opcode: 011 01 110010
                         //                  ; 0x632 = 011001110010
        
        /* 015 */ 0x000, // (no usado)
        /* 016 */ 0x000, // (no usado)
        /* 017 */ 0x000, // (no usado)
        
        // ============ SUBRUTINA: DOBLE ============
        // Duplica el valor en AC
        // Retorna el resultado en AC
        /* 018 */ 0x036, // ST.A, /36        ; Guardar parámetro
        /* 019 */ 0x236, // LD.A, /36        ; AC = parámetro
        /* 01A */ 0x436, // ADD.A, /36       ; AC = AC + AC (duplicar)
        
        // Retornar: saltar a la dirección guardada en mem[0x32]
        /* 01B */ 0x632, // BR, [/32]        ; Salto indirecto a dirección_retorno
        
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
        
        // ZONA DE VARIABLES
        /* 030 */ 0x000, // operando1
        /* 031 */ 0x000, // operando2
        /* 032 */ 0x000, // dirección_retorno (puntero de retorno)
        /* 033 */ 0x000, // resultado_suma
        /* 034 */ 0x000, // resultado_final
        /* 035 */ 0x000, // temp_suma
        /* 036 */ 0x000, // param_doble
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
    
    printf("FLUJO DE EJECUCIÓN:\n");
    printf("┌────────────────────────────────────────────────────────────┐\n");
    printf("│ 1. Inicialización: operando1=5, operando2=3               │\n");
    printf("│ 2. Llamada a subrutina_suma()                             │\n");
    printf("│    └─> Cálculo: 5 + 3 = 8                                 │\n");
    printf("│    └─> Retorno con resultado en AC                        │\n");
    printf("│ 3. Guardar resultado_suma = 8                             │\n");
    printf("│ 4. Llamada a subrutina_doble(8)                           │\n");
    printf("│    └─> Cálculo: 8 × 2 = 16                                │\n");
    printf("│    └─> Retorno con resultado en AC                        │\n");
    printf("│ 5. Guardar resultado_final = 16                           │\n");
    printf("│ 6. HALT                                                    │\n");
    printf("└────────────────────────────────────────────────────────────┘\n");
    printf("\n");
    
    printf("VALORES EN MEMORIA:\n");
    printf("  mem[0x30] = %d (operando1)\n", mem[0x30]);
    printf("  mem[0x31] = %d (operando2)\n", mem[0x31]);
    printf("  mem[0x32] = 0x%03X (última dir. retorno guardada)\n", mem[0x32]);
    printf("  mem[0x33] = %d (resultado suma: 5+3)\n", mem[0x33]);
    printf("  mem[0x34] = %d (resultado final: 8×2)\n", mem[0x34]);
    printf("\n");
    
    if (mem[0x33] == 8 && mem[0x34] == 16) {
        printf("✓ ÉXITO: Las subrutinas funcionaron correctamente!\n");
        printf("  suma(5, 3) = 8\n");
        printf("  doble(8) = 16\n");
    } else {
        printf("✗ NOTA: Los resultados difieren de lo esperado\n");
    }
    printf("\n");
    
    printf("CONCEPTO APRENDIDO:\n");
    printf("  - Las subrutinas permiten código reutilizable y modular\n");
    printf("  - Se necesita guardar la dirección de retorno antes del salto\n");
    printf("  - El salto indirecto (BR [/dir]) permite retornar dinámicamente\n");
    printf("  - En CPUs reales, esto se implementa con una pila (stack)\n");
    printf("\n");
    
    printf("COMPARACIÓN CON LENGUAJES DE ALTO NIVEL:\n");
    printf("/dev/null/comparison.c#L1-10\n");
    printf("  // En C:\n");
    printf("  int suma(int a, int b) { return a + b; }\n");
    printf("  int doble(int x) { return x * 2; }\n");
    printf("  \n");
    printf("  int main() {\n");
    printf("      int resultado = doble(suma(5, 3));\n");
    printf("      return 0;\n");
    printf("  }\n");
    printf("\n");
    
    printf("LIMITACIONES EN SIMPLEZ-13:\n");
    printf("  - No hay instrucción CALL/RET dedicada\n");
    printf("  - No hay stack pointer automático\n");
    printf("  - Solo podemos simular una llamada a la vez (no recursión)\n");
    printf("  - Los parámetros se pasan por memoria global\n");
    printf("\n");
    
    printf("EN CPUs MODERNAS:\n");
    printf("  - Instrucciones CALL (guarda PC en stack y salta)\n");
    printf("  - Instrucciones RET (recupera PC del stack)\n");
    printf("  - Stack pointer (SP) para gestión automática\n");
    printf("  - Registros para pasar parámetros\n");
    printf("  - Soporte para recursión y llamadas anidadas\n");
    printf("\n");
    
    return 0;
}