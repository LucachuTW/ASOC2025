#include "kernel.h"

// =================== VARIABLES GLOBALES ===================
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

// Símbolos definidos por el linker
extern uint32_t _start;
extern uint32_t _end;

// =================== FUNCIONES AUXILIARES ===================
static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

static void update_cursor() {
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        cursor_y = 0;
    }
}

// =================== FUNCIONES PRINCIPALES ===================

void clear_screen(void) {
    uint16_t blank = vga_entry(' ', COLOR_BLACK);
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

void putchar(char c, uint8_t color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c >= ' ') {
        int offset = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[offset] = vga_entry(c, color);
        cursor_x++;
    }
    
    update_cursor();
}

void print_string(const char* str, uint8_t color) {
    while (*str) {
        putchar(*str++, color);
    }
}

void print_string_at(const char* str, uint8_t color, int x, int y) {
    int old_x = cursor_x;
    int old_y = cursor_y;
    
    cursor_x = x;
    cursor_y = y;
    
    print_string(str, color);
    
    cursor_x = old_x;
    cursor_y = old_y;
}

void print_colored_line(const char* str, uint8_t bg, uint8_t fg) {
    uint8_t color = VGA_COLOR(bg, fg);
    print_string(str, color);
    putchar('\n', color);
}

void print_line(uint8_t color) {
    putchar('\n', color);
}

static void print_hex_digit(uint8_t value) {
    const char* hex_chars = "0123456789ABCDEF";
    putchar(hex_chars[value & 0x0F], COLOR_WHITE);
}

void print_hex(uint32_t value) {
    print_string("0x", COLOR_WHITE);
    
    for (int i = 28; i >= 0; i -= 4) {
        print_hex_digit((value >> i) & 0x0F);
    }
}

void print_dec(uint32_t value) {
    if (value == 0) {
        putchar('0', COLOR_WHITE);
        return;
    }
    
    char buffer[11];
    int i = 10;
    
    while (value > 0 && i > 0) {
        buffer[--i] = '0' + (value % 10);
        value /= 10;
    }
    
    while (i < 10) {
        putchar(buffer[i++], COLOR_WHITE);
    }
}

uint32_t get_kernel_size(void) {
    return (uint32_t)((unsigned int)&_end - (unsigned int)&_start);
}

// =================== FUNCIÓN PRINCIPAL ===================
void kmain(void) {
    clear_screen();
    
    // Banner principal con colores
    print_colored_line("================================================================================", COLOR_CYAN, COLOR_BLACK);
    print_colored_line("                         SISTEMA OPERATIVO BASICO                               ", COLOR_CYAN, COLOR_WHITE);
    print_colored_line("                              Bootloader x86                                    ", COLOR_CYAN, COLOR_WHITE);
    print_colored_line("================================================================================", COLOR_CYAN, COLOR_BLACK);
    print_line(COLOR_WHITE);
    
    // Información del kernel
    print_string("[", COLOR_GREEN);
    print_string("OK", COLOR_GREEN);
    print_string("] ", COLOR_GREEN);
    print_string("Kernel iniciado correctamente\n", COLOR_WHITE);
    
    print_string("[", COLOR_GREEN);
    print_string("OK", COLOR_GREEN);
    print_string("] ", COLOR_GREEN);
    print_string("Modo protegido activado (32-bit)\n", COLOR_WHITE);
    
    print_string("[", COLOR_GREEN);
    print_string("OK", COLOR_GREEN);
    print_string("] ", COLOR_GREEN);
    print_string("GDT cargada y configurada\n", COLOR_WHITE);
    
    print_line(COLOR_WHITE);
    
    // Información de memoria
    print_colored_line("--- MAPA DE MEMORIA ---", COLOR_YELLOW, COLOR_BLACK);
    
    print_string("  Stage1 (bootloader): ", COLOR_LIGHT_GRAY);
    print_hex(0x00007C00);
    print_string(" - ", COLOR_LIGHT_GRAY);
    print_hex(0x00007DFF);
    print_string(" (512 bytes)\n", COLOR_LIGHT_GRAY);
    
    print_string("  Stage2 (loader):     ", COLOR_LIGHT_GRAY);
    print_hex(0x00007E00);
    print_string(" - ", COLOR_LIGHT_GRAY);
    print_hex(0x00007FFF);
    print_string(" (1024 bytes)\n", COLOR_LIGHT_GRAY);
    
    print_string("  Kernel:              ", COLOR_LIGHT_GRAY);
    print_hex(0x00010000);
    print_string(" - ", COLOR_LIGHT_GRAY);
    print_hex((uint32_t)&_end);
    print_string(" (", COLOR_LIGHT_GRAY);
    print_dec(get_kernel_size());
    print_string(" bytes)\n", COLOR_LIGHT_GRAY);
    
    print_string("  Stack:               ", COLOR_LIGHT_GRAY);
    print_hex(0x00090000);
    print_string(" (crece hacia abajo)\n", COLOR_LIGHT_GRAY);
    
    print_string("  VGA Buffer:          ", COLOR_LIGHT_GRAY);
    print_hex(0x000B8000);
    print_string(" (80x25 texto)\n", COLOR_LIGHT_GRAY);
    
    print_line(COLOR_WHITE);
    
    // Información del sistema
    print_colored_line("--- INFORMACION DEL SISTEMA ---", COLOR_YELLOW, COLOR_BLACK);
    
    print_string("  Arquitectura:        ", COLOR_LIGHT_GRAY);
    print_string("x86 (i386)\n", COLOR_WHITE);
    
    print_string("  Modo CPU:            ", COLOR_LIGHT_GRAY);
    print_string("Protected Mode (32-bit)\n", COLOR_WHITE);
    
    print_string("  Tamano kernel:       ", COLOR_LIGHT_GRAY);
    print_dec(get_kernel_size());
    print_string(" bytes (", COLOR_WHITE);
    print_dec((get_kernel_size() + 511) / 512);
    print_string(" sectores)\n", COLOR_WHITE);
    
    print_string("  Direccion inicio:    ", COLOR_LIGHT_GRAY);
    print_hex(0x10000);
    print_string("\n", COLOR_WHITE);
    
    print_string("  Direccion fin:       ", COLOR_LIGHT_GRAY);
    print_hex((uint32_t)&_end);
    print_string("\n", COLOR_WHITE);
    
    print_line(COLOR_WHITE);
    
    // Mensaje final
    print_colored_line("================================================================================", COLOR_CYAN, COLOR_BLACK);
    print_string("  Sistema listo.  Virus Payal.\n", COLOR_LIGHT_CYAN);
    print_colored_line("================================================================================", COLOR_CYAN, COLOR_BLACK);
    
    // Bucle infinito con halt
    while(1) {
        __asm__ volatile("hlt");
    }
}