#ifndef KERNEL_H
#define KERNEL_H

// =================== TIPOS BÁSICOS ===================
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned int uintptr_t;

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

// Colores VGA (4 bits: 0-15)
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

// Macros para colores
#define VGA_COLOR(bg, fg) ((bg << 4) | fg)

// =================== FUNCIONES PÚBLICAS ===================
// =================== ESTRUCTURAS ===================
struct GDTR { uint16_t limit; uint32_t base; } __attribute__((packed));

// =================== FUNCIONES PRINCIPALES ===================
void kmain(void);

// =================== VIDEO / SALIDA ===================
void clear_screen(void);
void putchar(char c, unsigned char color);
void print_string(const char* str, unsigned char color);
void print_string_at(const char* str, unsigned char color, int x, int y);
void print_separator(char ch, unsigned char attr);
void print_centered(const char* s, unsigned char attr);
void print_kv(const char* key, const char* val, unsigned char ka, unsigned char va);
void print_status_checked(const char* text, bool ok);
void newline(unsigned char attr); // expone salto de línea controlado
void print_hex(uint32_t value);
void print_dec(uint32_t value);
uint32_t get_kernel_size(void);

// =================== UTILIDADES KERNEL ===================
uint32_t kernel_checksum(void);
void countdown(int seconds);
bool detect_optional_module(void);
void show_module_status(void);

// =================== COMPROBACIONES HW ===================
bool check_protected_mode(void);
bool check_segments_ok(void);
bool check_gdt_ok(struct GDTR* out);
bool check_a20_enabled(void);
bool check_vga_access(void);
uint32_t read_esp(void);

// Direcciones compartidas con stage2
#define STAGE2_BASE          0x00007E00  // Inicio de stage2 en memoria
#define STAGE2_DISK_OK       (STAGE2_BASE + 0) // byte de estado lectura kernel
#define STAGE2_KERNEL_SECTS  (STAGE2_BASE + 1) // sectores kernel
#define STAGE2_MODULE_OK     (STAGE2_BASE + 2) // módulo opcional leído
#define STAGE2_MODULE_SECTS  (STAGE2_BASE + 3) // sectores módulo opcional

// Dirección donde stage2 intenta cargar módulo opcional
#define MODULE_LOAD_ADDRESS  0x00012000

// =================== INLINES ===================
static inline unsigned char vga_attr(unsigned char bg, unsigned char fg) { return VGA_COLOR(bg, fg); }


#endif