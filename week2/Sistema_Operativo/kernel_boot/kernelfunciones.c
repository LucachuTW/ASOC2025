#include "kernel.h"

// Archivo que agrupa las funciones auxiliares del pseudokernel.
// Mantiene kernel.c centrado en kmain.

// =================== ESTADO INTERNO ===================
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

extern uint32_t _start; // símbolos del linker
extern uint32_t _end;

// =================== AUXILIARES PRIVADOS ===================
void newline(uint8_t attr) {
    // Limpiar hasta final de línea para evitar residuos visuales
    int off = cursor_y * VGA_WIDTH + cursor_x;
    for (int i = cursor_x; i < VGA_WIDTH; ++i) vga_buffer[off + (i - cursor_x)] = (uint16_t)(' ') | ((uint16_t)attr << 8);
    cursor_x = 0; cursor_y++; if (cursor_y >= VGA_HEIGHT) cursor_y = 0;
}

static inline uint16_t vga_entry(char c, uint8_t attr) { return (uint16_t)c | (uint16_t)attr << 8; }

static void print_hex32_attr(uint32_t v, uint8_t a) {
    const char* hex = "0123456789ABCDEF";
    print_string("0x", a);
    for (int i = 28; i >= 0; i -= 4) putchar(hex[(v >> i) & 0xF], a);
}

static void print_dec_attr(uint32_t value, uint8_t a) {
    if (value == 0) { putchar('0', a); return; }
    char buf[12]; int i = 0;
    while (value > 0 && i < (int)sizeof(buf)-1) { buf[i++] = '0' + (value % 10); value /= 10; }
    for (int j = i - 1; j >= 0; --j) putchar(buf[j], a);
}

// =================== VIDEO / SALIDA ===================
void clear_screen(void) {
    uint16_t blank = vga_entry(' ', vga_attr(THEME_BG, THEME_MUTED));
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) vga_buffer[i] = blank;
    cursor_x = 0; cursor_y = 0;
}

void putchar(char c, unsigned char attr) {
    if (c == '\n') { newline(attr); return; }
    if (c == '\t') { cursor_x = (cursor_x + 8) & ~7; return; }
    if (c < ' ') return;
    int offset = cursor_y * VGA_WIDTH + cursor_x;
    vga_buffer[offset] = vga_entry(c, attr);
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) newline(attr);
}

void print_string(const char* s, unsigned char a) { while (*s) putchar(*s++, a); }

void print_string_at(const char* s, unsigned char a, int x, int y) {
    int ox = cursor_x, oy = cursor_y;
    cursor_x = x; cursor_y = y; print_string(s, a);
    cursor_x = ox; cursor_y = oy;
}

void print_separator(char ch, unsigned char a) {
    for (int i = 0; i < VGA_WIDTH; ++i) putchar(ch, a);
    newline(a);
}

void print_centered(const char* s, unsigned char a) {
    int len = 0; for (const char* p = s; *p; ++p) ++len;
    int x = (VGA_WIDTH - len) / 2; if (x < 0) x = 0;
    print_string_at(s, a, x, cursor_y);
    newline(a);
}

void print_kv(const char* key, const char* val, unsigned char ka, unsigned char va) {
    const int keycol = 24;
    print_string("  ", ka);
    print_string(key, ka);
    int cur = cursor_x;
    while (cur++ < keycol) putchar(' ', ka);
    print_string(val, va);
    newline(va);
}

void print_status_checked(const char* text, bool ok) {
    uint8_t gray = vga_attr(THEME_BG, THEME_MUTED);
    uint8_t okc  = vga_attr(THEME_BG, THEME_OK);
    uint8_t errc = vga_attr(THEME_BG, THEME_ERR);
    uint8_t white= vga_attr(THEME_BG, THEME_TEXT);
    print_string("[", gray); putchar(' ', gray);
    print_string(ok ? "OK" : "ERR", ok ? okc : errc); putchar(' ', gray);
    print_string("] ", gray);
    print_string(text, white);
    newline(white);
}

void print_hex(uint32_t v) { print_hex32_attr(v, vga_attr(THEME_BG, THEME_TEXT)); }
void print_dec(uint32_t v) { print_dec_attr(v, vga_attr(THEME_BG, THEME_TEXT)); }

uint32_t get_kernel_size(void) { return (uint32_t)((unsigned int)&_end - (unsigned int)&_start); }

// =================== COMPROBACIONES HW ===================
static inline uint32_t read_cr0(void) { uint32_t v; __asm__ volatile("mov %%cr0, %0" : "=r"(v)); return v; }
static inline uint16_t read_cs(void) { uint16_t v; __asm__ volatile("mov %%cs, %0" : "=r"(v)); return v; }
static inline uint16_t read_ds(void) { uint16_t v; __asm__ volatile("mov %%ds, %0" : "=r"(v)); return v; }
static inline uint16_t read_ss(void) { uint16_t v; __asm__ volatile("mov %%ss, %0" : "=r"(v)); return v; }
uint32_t read_esp(void) { uint32_t v; __asm__ volatile("mov %%esp, %0" : "=r"(v)); return v; }
static inline uint8_t inb(uint16_t port) { uint8_t v; __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port)); return v; }
static inline void read_gdtr_internal(struct GDTR* g) { __asm__ volatile("sgdt %0" : "=m"(*g)); }

bool check_protected_mode(void) { return (read_cr0() & 0x1u) != 0; }
bool check_segments_ok(void) { return (read_cs() == 0x08) && (read_ds() == 0x10) && (read_ss() == 0x10); }
bool check_gdt_ok(struct GDTR* out) { struct GDTR g; read_gdtr_internal(&g); if (out) *out = g; return g.base != 0 && g.limit >= 23; }
bool check_a20_enabled(void) { return (inb(0x92) & 0x02) != 0; }
bool check_vga_access(void) { volatile uint16_t* v = (volatile uint16_t*)VGA_ADDRESS; uint16_t i=79, old=v[i], test=0x2F56; v[i]=test; uint16_t rb=v[i]; v[i]=old; return rb==test; }

// =================== CHECKSUM ===================
uint32_t kernel_checksum(void) {
    uint8_t* p = (uint8_t*)&_start; uint8_t* e = (uint8_t*)&_end; uint32_t sum=0; while (p<e) sum += *p++; return sum;
}

// =================== MÓDULO OPCIONAL ===================
bool detect_optional_module(void) {
    volatile uint8_t* status = (volatile uint8_t*)STAGE2_BASE;
    return status[2] ? true : false;
}

void show_module_status(void) {
    volatile uint8_t* status = (volatile uint8_t*)STAGE2_BASE;
    uint8_t disk = status[0];
    uint8_t ksects = status[1];
    uint8_t mod = status[2];
    uint8_t msects = status[3];
    uint8_t a_hdr = vga_attr(THEME_BG, THEME_SECONDARY);
    print_separator('~', a_hdr);
    print_centered("ESTADO DE CARGA", vga_attr(THEME_BG, THEME_TEXT));
    print_separator('~', a_hdr);
    print_kv("Disco kernel:", disk ? "OK" : "FALLO", vga_attr(THEME_BG, THEME_MUTED), disk ? vga_attr(THEME_BG, THEME_OK) : vga_attr(THEME_BG, THEME_ERR));
    // sectores kernel como decimal de 2 cifras
    char sbuf[3]; sbuf[0] = (ksects/10)? ('0'+ (ksects/10)) : '0'; sbuf[1] = '0'+ (ksects%10); sbuf[2] = 0;
    print_kv("Sectores kernel:", sbuf, vga_attr(THEME_BG, THEME_MUTED), vga_attr(THEME_BG, THEME_TEXT));
    print_kv("Modulo opcional:", mod ? "CARGADO" : "NO", vga_attr(THEME_BG, THEME_MUTED), mod ? vga_attr(THEME_BG, THEME_OK) : vga_attr(THEME_BG, THEME_ERR));
    if (mod) print_kv("Sectores modulo:", msects==1?"1":"?", vga_attr(THEME_BG, THEME_MUTED), vga_attr(THEME_BG, THEME_TEXT));
}

// =================== COUNTDOWN ===================
void countdown(int seconds) {
    for (int s = seconds; s > 0; --s) {
        clear_screen();
        uint8_t bg = (s >= 3) ? THEME_ERR : (s==2 ? THEME_ACCENT : THEME_OK);
        uint8_t a = vga_attr(bg, THEME_TEXT);
        print_separator('=', a);
        print_centered("INICIANDO PSEUDOKERNEL", a);
        print_centered("Cuenta atras", a);
        char msg[2] = { (char)('0'+s), 0 }; print_centered(msg, a);
        print_separator('=', a);
        for (volatile uint32_t i=0;i<1500000;++i) __asm__ volatile("nop");
    }
}
