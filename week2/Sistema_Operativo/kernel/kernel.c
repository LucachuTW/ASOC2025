#include "kernel.h"

// =================== VARIABLES GLOBALES ===================
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

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

// =================== FUNCIÓN PRINCIPAL ===================
void kmain(void) {
    clear_screen();
    
    // Banner del sistema
    print_string_at("╔═══════════════════════════════════════════════════════════════╗", COLOR_LIGHT_CYAN, 0, 0);
    print_string_at("║                     VIRUS PAYAL OS v1.0                      ║", COLOR_LIGHT_CYAN, 0, 1);
    print_string_at("╚═══════════════════════════════════════════════════════════════╝", COLOR_LIGHT_CYAN, 0, 2);
    
    cursor_y = 4;
    
    print_string("✓ Sistema iniciado correctamente\n", COLOR_LIGHT_GREEN);
    print_string("✓ Modo protegido activado\n", COLOR_LIGHT_GREEN);
    print_string("✓ Subsistema VGA inicializado\n", COLOR_LIGHT_GREEN);
    
    print_line(COLOR_WHITE);
    
    print_string("Memoria kernel: ", COLOR_LIGHT_GRAY);
    print_hex(0x8000);
    print_string(" - ", COLOR_WHITE);
    print_hex(0x9000);
    print_line(COLOR_WHITE);
    
    print_string("Tamaño pantalla: ", COLOR_LIGHT_GRAY);
    print_dec(VGA_WIDTH);
    print_string("x", COLOR_WHITE);
    print_dec(VGA_HEIGHT);
    print_string(" caracteres\n", COLOR_WHITE);
    
    print_line(COLOR_WHITE);
    
    print_string("> ", COLOR_YELLOW);
    
    // Bucle principal del kernel
    while (1) {
        __asm__ volatile("hlt");
    }
}