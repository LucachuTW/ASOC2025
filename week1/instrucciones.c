#include "simplez.h"

// --- Implementación del Repertorio de Instrucciones ---
// Basado en la tabla de la página 20 

// NOTA: El PDF indica R=0 es AC, R=1 es X [cite: 106, 107, 278, 279]

void st(uint8_t reg, uint16_t ea) {
    if (reg == 0) { // R=0 -> AC
        mem[ea] = acc;
    } else { // R=1 -> X
        mem[ea] = x;
    }
}

void ld(uint8_t reg, uint16_t ea) {
    if (reg == 0) { // R=0 -> AC
        acc = mem[ea];
        status.z = (acc == 0); // Actualiza bit Z [cite: 293]
    } else { // R=1 -> X
        x = mem[ea];
        status.z = (x == 0); // Actualiza bit Z [cite: 293]
    }
}

void add(uint8_t reg, uint16_t ea) {
    if (reg == 0) { // R=0 -> AC
        acc += mem[ea];
        status.z = (acc == 0); // Actualiza bit Z [cite: 293]
    } else { // R=1 -> X
        x += mem[ea];
        status.z = (x == 0); // Actualiza bit Z [cite: 293]
    }
}

void br(uint8_t reg, uint16_t ea) {
    // El bit 8 (R) es indiferente [cite: 313]
    pc = ea;
    pc--; // Compensamos el pc++ al final del bucle loop()
}

void bz(uint8_t reg, uint16_t ea) {
    // El bit 8 (R) es indiferente [cite: 313]
    if (status.z) { // Salta solo si el bit Z es 1 [cite: 291]
        pc = ea;
        pc--; // Compensamos el pc++
    }
}

void ld_sharp(uint8_t reg, uint16_t literal) {
    // Modo de direccionamiento siempre inmediato [cite: 315]
    if (reg == 0) { // R=0 -> AC
        acc = literal;
        status.z = (acc == 0); // Actualiza bit Z [cite: 293]
    } else { // R=1 -> X
        x = literal;
        status.z = (x == 0); // Actualiza bit Z [cite: 293]
    }
}

void sub_sharp(uint8_t reg, uint16_t literal) {
    // Modo de direccionamiento siempre inmediato [cite: 315]
    if (reg == 0) { // R=0 -> AC
        acc -= literal;
        status.z = (acc == 0); // Actualiza bit Z [cite: 293]
    } else { // R=1 -> X
        x -= literal;
        status.z = (x == 0); // Actualiza bit Z [cite: 293]
    }
}

void halt(uint8_t reg, uint16_t literal) {
    status.h = 1; // Activa el flag de Halt
    pc--; // Compensamos el pc++ para que no se pase
}