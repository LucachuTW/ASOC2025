#ifndef KERNEL_H
#define KERNEL_H

// =================== TIPOS BÁSICOS ===================
#include <stdint.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#include "module_info.h"
#include "elf.h"

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

// =================== TEMA (paleta coherente) ===================
#define THEME_BG        COLOR_BLACK
#define THEME_TEXT      COLOR_WHITE
#define THEME_MUTED     COLOR_LIGHT_GRAY
#define THEME_PRIMARY   COLOR_LIGHT_CYAN
#define THEME_SECONDARY COLOR_LIGHT_MAGENTA
#define THEME_ACCENT    COLOR_YELLOW
#define THEME_OK        COLOR_LIGHT_GREEN
#define THEME_ERR       COLOR_LIGHT_RED

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
void newline(unsigned char attr); // salto de línea que además limpia hasta fin de línea
void print_hex(uint32_t value);
void print_dec(uint32_t value);
uint32_t get_kernel_size(void);

// =================== UTILIDADES KERNEL ===================
uint32_t kernel_checksum(void);
void countdown(int seconds);
void progress_bar_ms(int total_ms, const char* label);
void progress_bar_inline_ms(int total_ms, int width);
// Nueva API para barra en dos fases
void progress_bar_inline_draw(int width, int* out_x, int* out_y, int* out_fill);
void progress_bar_inline_animate(int total_ms, int x, int y, int fill);
// Acceso cursor para control preciso
void get_cursor(int* x, int* y);
void set_cursor(int x, int y);
// Espera a que se pulse cualquier tecla o a timeout (ms). Devuelve true si hubo tecla.
bool wait_key_or_timeout_ms(int timeout_ms);
// Mantener prototipos del kernel (usar stdbool/stdint)
bool wait_for_keypress(void);
// Vaciar el buffer del controlador de teclado 8042
void kbd_flush(void);
bool detect_optional_module(void);
void show_module_status(void);
// Sonda previa del módulo (lee a buffer interno y valida magic). Finalizar copia tras barra.
bool module_probe(void);
bool module_finalize_after_bar(void);
void* module_get_entry(void);

// ATA PIO mínimo (28-bit LBA, 1 sector)
int ata_read28(uint32_t lba, void* dst);

// =================== UTILIDADES DE FORMATO/ANALISIS ===================
// Imprime n bytes en hexadecimal (dos dígitos) separados por espacio
void print_hex_bytes(const uint8_t* p, int n, unsigned char attr_hex, unsigned char attr_sep);

// =================== COMPROBACIONES HW ===================
bool check_protected_mode(void);
bool check_segments_ok(void);
bool check_gdt_ok(struct GDTR* out);
bool check_a20_enabled(void);
bool check_vga_access(void);
uint32_t read_esp(void);

// Direcciones compartidas con stage2 (header verificable)
// stage2 escribe un header verificable en 0x7E00 tras cargar el kernel:
// [0..1] word MAGIC = 0x5453 ('S''T'), [2] version, [3] disk, [4] kernel_sects
#define STAGE2_BASE          0x00007E00
#define STAGE2_MAGIC         (STAGE2_BASE + 0)  /* word */
#define STAGE2_VERSION       (STAGE2_BASE + 2)  /* byte */
#define STAGE2_DISK          (STAGE2_BASE + 3)  /* byte */
#define STAGE2_KERNEL_SECTS  (STAGE2_BASE + 4)  /* byte */

// Compatibilidad con nombres previos (mapeamos al nuevo header)
#define STAGE2_DISK_OK       STAGE2_DISK
#undef STAGE2_MODULE_OK
#define STAGE2_MODULE_OK     (STAGE2_BASE + 5)
#undef STAGE2_MODULE_SECTS
#define STAGE2_MODULE_SECTS  (STAGE2_BASE + 6)

// Dirección donde stage2 intenta cargar módulo opcional
// Si `kernel_boot/module_info.h` (generado por Makefile) define `MODULE_LOAD_ADDRESS`,
// lo usamos. Si no existe, dejamos un valor por defecto para desarrollo.
#ifndef MODULE_LOAD_ADDRESS
/*  0x00120000; sin embargo, el Makefile permite cambiar esta
	variable al compilar (p. ej. `make MODULE_LOAD_ADDRESS=0x00200000`). Si quieres
	estar aún más seguro contra colisiones con BIOS/VGA, usa una dirección >1MB. */
#define MODULE_LOAD_ADDRESS  0x00120000
#endif

// =================== INLINES ===================
static inline unsigned char vga_attr(unsigned char bg, unsigned char fg) { return VGA_COLOR(bg, fg); }


#endif
