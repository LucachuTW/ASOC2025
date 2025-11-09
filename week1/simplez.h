#ifndef SIMPLEZ_H
#define SIMPLEZ_H

#include <stdint.h>
#include <stdio.h>

// --- Definición del "Hardware" ---

// Capacidad de la Memoria Principal (MP) de 12 bits de bus A (2^12)
// [cite: 39]
#define MEM_SIZE 4096

// Memoria Principal
extern uint16_t mem[MEM_SIZE];

// Registros de la UCP [cite: 38]
extern uint16_t acc; // Acumulador (AC)
extern uint16_t x;   // Registro de Índice (X)
extern uint16_t pc;  // Contador de Programa (PC)

// Registro de Estado [cite: 45]
struct status {
    uint8_t z : 1; // Bit Z (Cero). 1 si la última op. UAL fue 0 [cite: 45]
    uint8_t h : 1; // Bit H (Halt). 1 para detener el emulador
};
extern struct status status;


// --- Declaraciones de Funciones (Prototipos) ---

// --- Funciones del Núcleo de la CPU (en main.c) ---

/**
 * @brief Inicia el ciclo principal de la CPU (Fetch-Decode-Execute).
 */
void loop();

// --- Funciones del Repertorio de Instrucciones (en instrucciones.c) ---
// 

// R=0 opera con AC, R=1 opera con X [cite: 106, 107]

/**
 * @brief 000 - ST: Almacena AC o X en memoria. (AC) -> MP[DE] ó (X) -> MP[DE]
 * @param reg Registro (0=AC, 1=X)
 * @param ea Dirección Efectiva (DE) donde almacenar
 */
void st(uint8_t reg, uint16_t ea);

/**
 * @brief 001 - LD: Carga AC o X desde memoria. (MP[DE]) -> AC ó (MP[DE]) -> X
 * @param reg Registro (0=AC, 1=X)
 * @param ea Dirección Efectiva (DE) de donde leer
 */
void ld(uint8_t reg, uint16_t ea);

/**
 * @brief 010 - ADD: Suma a AC o X. (AC)+(MP[DE]) -> AC ó (X)+(MP[DE]) -> X
 * @param reg Registro (0=AC, 1=X)
 * @param ea Dirección Efectiva (DE) del operando
 */
void add(uint8_t reg, uint16_t ea);

/**
 * @brief 011 - BR: Salto incondicional. Siguiente instrucción en MP[DE]
 * @param reg No usado (indiferente) [cite: 313]
 * @param ea Dirección Efectiva (DE) a la que saltar
 */
void br(uint8_t reg, uint16_t ea);

/**
 * @brief 100 - BZ: Salto si Z=1. Si cero en UAL, sig. instr. en MP[DE]
 * @param reg No usado (indiferente) [cite: 313]
 * @param ea Dirección Efectiva (DE) a la que saltar
 */
void bz(uint8_t reg, uint16_t ea);

/**
 * @brief 101 - LD #: Carga Inmediata (literal). (CD) -> AC ó (CD) -> X
 * @param reg Registro (0=AC, 1=X)
 * @param literal Valor inmediato (de 6 bits) a cargar
 */
void ld_sharp(uint8_t reg, uint16_t literal);

/**
 * @brief 110 - SUB #: Resta Inmediata (literal). (AC)-(CD) -> AC ó (X)-(CD) -> X
 * @param reg Registro (0=AC, 1=X)
 * @param literal Valor inmediato (de 6 bits) a restar
 */
void sub_sharp(uint8_t reg, uint16_t literal);

/**
 * @brief 111 - HALT: Detiene la ejecución.
 * @param reg No usado
 * @param literal No usado
 */
void halt(uint8_t reg, uint16_t literal);


#endif //SIMPLEZ_H