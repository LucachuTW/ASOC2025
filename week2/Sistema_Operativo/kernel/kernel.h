#ifndef KERNEL_H
#define KERNEL_H

// =================== TIPOS BÁSICOS ===================
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#if __STDC_VERSION__ < 202000L
    #define bool _Bool
    #define true 1
    #define false 0
#endif

#define NULL ((void*)0)

// =================== VIDEO VGA ===================
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Colores VGA simplificados (combinados bg << 4 | fg)
#define COLOR_BLACK         0x0
#define COLOR_BLUE          0x1
#define COLOR_GREEN         0x2
#define COLOR_CYAN          0x3
#define COLOR_RED           0x4
#define COLOR_MAGENTA       0x5
#define COLOR_BROWN         0x6
#define COLOR_LIGHT_GRAY    0x7
#define COLOR_DARK_GRAY     0x8
#define COLOR_LIGHT_BLUE    0x9
#define COLOR_LIGHT_GREEN   0xA
#define COLOR_LIGHT_CYAN    0xB
#define COLOR_LIGHT_RED     0xC
#define COLOR_LIGHT_MAGENTA 0xD
#define COLOR_YELLOW        0xE
#define COLOR_WHITE         0xF

// =================== FUNCIONES PÚBLICAS ===================
void kmain(void);
void clear_screen(void);
void putchar(char c, unsigned char color);
void print_string(const char* str, unsigned char color);
void print_colored(const char* str, unsigned char bg, unsigned char fg);
void print_hex(unsigned int value);

#endif