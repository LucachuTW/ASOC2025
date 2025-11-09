#ifndef KERNEL_H
#define KERNEL_H

// Tipos básicos
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// Video
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Colores VGA
#define COLOR_BLACK 0x0
#define COLOR_WHITE 0x7
#define COLOR_GREEN 0x2
#define COLOR_RED 0x4

// Funciones
void kmain(void);
void clear_screen(void);
void print_string(const char* str, uint8_t color);
void print_hex(uint32_t value);

#endif