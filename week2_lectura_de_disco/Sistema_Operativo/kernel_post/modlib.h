#ifndef MODLIB_H
#define MODLIB_H
#include <stdint.h>

#define VGA_MEM ((volatile uint16_t*)0xB8000)
#define VGA_W 80
#define VGA_H 25

void cls(uint8_t attr);
void putc_xy(char c,int x,int y,uint8_t attr);
void print_xy(const char* s,int x,int y,uint8_t attr);
void delay(volatile uint32_t loops);
uint8_t inb(uint16_t port);
int key_pressed(void);
void fmt_hex(uint32_t v, char* out);

// Nuevos prototipos necesarios
void u32_to_dec(uint32_t v, char* out);
void print_label_hex(const char* label, uint32_t value, int y, uint8_t la, uint8_t va);
// Bloqueante: esperar tecla válida (versión del módulo, renombrada para evitar colisiones)
// ...existing code...
// Esperar tecla válida (versión del módulo, namespaced para evitar colisiones)
void mod_wait_for_keypress(void);


// Nuevo: vaciar scancodes pendientes (8042) - namespaced
void mod_kbd_flush(void);


#endif
