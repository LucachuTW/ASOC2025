#include "kernel.h"

// =================== VARIABLES PRIVADAS ===================
static volatile unsigned short* video_memory = (volatile unsigned short*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

// =================== FUNCIONES ===================

void clear_screen(void) {
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    unsigned short blank = 0x0F20;  // 0x0F = blanco sobre negro, 0x20 = espacio
    
    for (int i = 0; i < 2000; i++) {  // 80*25 = 2000
        vga[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

void putchar_direct(char c, unsigned char color, int x, int y) {
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    int offset = y * 80 + x;
    vga[offset] = (unsigned short)c | ((unsigned short)color << 8);
}

void print_string_simple(const char* str, unsigned char color) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            putchar_direct(str[i], color, cursor_x, cursor_y);
            cursor_x++;
            if (cursor_x >= 80) {
                cursor_x = 0;
                cursor_y++;
            }
        }
        
        if (cursor_y >= 25) {
            cursor_y = 0;
        }
        
        i++;
    }
}

void kmain(void) {
    // Test 1: Clear screen
    clear_screen();
    
    // Test 2: Una línea simple
    print_string_simple("Virus Payal OS v1.0\n", 0x0F);
    
    // Test 3: Más líneas
    print_string_simple("Sistema iniciado OK\n", 0x0A);
    print_string_simple("> ", 0x0B);
    
    // Bucle infinito
    while(1) {
        __asm__ volatile("hlt");
    }
}