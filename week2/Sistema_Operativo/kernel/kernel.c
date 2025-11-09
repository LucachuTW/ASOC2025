// Reescritura limpia con comprobaciones reales y mejoras estéticas
#include "kernel.h"

// =================== VARIABLES GLOBALES ===================
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

// Símbolos definidos por el linker
extern uint32_t _start;
extern uint32_t _end;

// =================== AUXILIARES DE VIDEO ===================
static inline uint16_t vga_entry(char c, uint8_t attr) {
    return (uint16_t)c | (uint16_t)attr << 8;
}

static inline uint8_t attr(uint8_t bg, uint8_t fg) { return VGA_COLOR(bg, fg); }

void clear_screen(void) {
    uint16_t blank = vga_entry(' ', attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) vga_buffer[i] = blank;
    cursor_x = 0; cursor_y = 0;
}

static void newline(uint8_t a) { cursor_x = 0; cursor_y++; if (cursor_y >= VGA_HEIGHT) cursor_y = 0; }

void putchar(char c, uint8_t a) {
    if (c == '\n') { newline(a); return; }
    if (c == '\t') { cursor_x = (cursor_x + 8) & ~7; return; }
    if (c < ' ') return;
    int offset = cursor_y * VGA_WIDTH + cursor_x;
    vga_buffer[offset] = vga_entry(c, a);
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) newline(a);
}

void print_string(const char* s, uint8_t a) { while (*s) putchar(*s++, a); }

void print_string_at(const char* s, uint8_t a, int x, int y) {
    int ox = cursor_x, oy = cursor_y;
    cursor_x = x; cursor_y = y; print_string(s, a);
    cursor_x = ox; cursor_y = oy;
}

// Llena toda la línea con un caracter (para separadores/color blocks)
void print_separator(char ch, uint8_t a) {
    for (int i = 0; i < VGA_WIDTH; i++) putchar(ch, a);
    newline(a);
}

// Imprime texto centrado en la línea actual
void print_centered(const char* s, uint8_t a) {
    int len = 0; for (const char* p = s; *p; ++p) ++len;
    int x = (VGA_WIDTH - len) / 2; if (x < 0) x = 0;
    print_string_at(s, a, x, cursor_y);
    newline(a);
}

// Imprime una pareja clave:valor alineada en 2 columnas
void print_kv(const char* key, const char* val, uint8_t ka, uint8_t va) {
    const int keycol = 24; // ancho reservado para la clave
    print_string("  ", ka);
    print_string(key, ka);
    int cur = cursor_x;
    if (cur < keycol) {
        for (int i = cur; i < keycol; ++i) putchar(' ', ka);
    }
    print_string(val, va);
    newline(va);
}

// ======== Formateo de números ========
static void print_hex32(uint32_t v, uint8_t a) {
    const char* hex = "0123456789ABCDEF";
    print_string("0x", a);
    for (int i = 28; i >= 0; i -= 4) {
        putchar(hex[(v >> i) & 0xF], a);
    }
}

// impresión decimal con atributo (función interna)
static void print_dec_attr(uint32_t value, uint8_t a) {
    if (value == 0) { putchar('0', a); return; }
    char buf[12]; int i = 0; while (value > 0 && i < (int)sizeof(buf)-1) { buf[i++] = '0' + (value % 10); value /= 10; }
    for (int j = i-1; j >= 0; --j) putchar(buf[j], a);
}

// Wrappers públicos (coinciden con prototipos en kernel.h)
void print_hex(uint32_t v) { print_hex32(v, attr(COLOR_BLACK, COLOR_WHITE)); }
void print_dec(uint32_t v) { print_dec_attr(v, attr(COLOR_BLACK, COLOR_WHITE)); }

uint32_t get_kernel_size(void) { return (uint32_t)((unsigned int)&_end - (unsigned int)&_start); }

// ======== Lecturas de CPU / HW (inline asm) ========
static inline uint32_t read_cr0(void) { uint32_t v; __asm__ volatile("mov %%cr0, %0" : "=r"(v)); return v; }
static inline uint16_t read_cs(void) { uint16_t v; __asm__ volatile("mov %%cs, %0" : "=r"(v)); return v; }
static inline uint16_t read_ds(void) { uint16_t v; __asm__ volatile("mov %%ds, %0" : "=r"(v)); return v; }
static inline uint16_t read_ss(void) { uint16_t v; __asm__ volatile("mov %%ss, %0" : "=r"(v)); return v; }
static inline uint32_t read_esp(void) { uint32_t v; __asm__ volatile("mov %%esp, %0" : "=r"(v)); return v; }
static inline uint8_t inb(uint16_t port) { uint8_t v; __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port)); return v; }
struct GDTR { uint16_t limit; uint32_t base; } __attribute__((packed));
static inline void read_gdtr(struct GDTR* g) { __asm__ volatile("sgdt %0" : "=m"(*g)); }

// ======== Comprobaciones reales ========
static bool check_protected_mode(void) { return (read_cr0() & 0x1u) != 0; }
static bool check_segments_ok(void) { return (read_cs() == 0x08) && (read_ds() == 0x10) && (read_ss() == 0x10); }
static bool check_gdt_ok(struct GDTR* out) { struct GDTR g; read_gdtr(&g); if (out) *out = g; return g.base != 0 && g.limit >= 23; }
static bool check_a20_enabled(void) { return (inb(0x92) & 0x02) != 0; }
static bool check_vga_access(void) { volatile uint16_t* vga = (volatile uint16_t*)VGA_ADDRESS; uint16_t i=79, old=vga[i], test=0x2F56; vga[i]=test; uint16_t rb=vga[i]; vga[i]=old; return rb==test; }

static void print_status_checked(const char* text, bool ok) {
    uint8_t gray = attr(COLOR_BLACK, COLOR_LIGHT_GRAY);
    uint8_t okc  = attr(COLOR_BLACK, COLOR_LIGHT_GREEN);
    uint8_t errc = attr(COLOR_BLACK, COLOR_LIGHT_RED);
    uint8_t white= attr(COLOR_BLACK, COLOR_WHITE);
    print_string("[", gray);
    putchar(' ', gray);
    print_string(ok ? "OK" : "ERR", ok ? okc : errc);
    putchar(' ', gray);
    print_string("] ", gray);
    print_string(text, white);
    newline(white);
}

// =================== FUNCIÓN PRINCIPAL ===================
void kmain(void) {
    clear_screen();

    // Banner principal
    uint8_t title_attr = attr(COLOR_CYAN, COLOR_BLACK);
    print_separator('=', title_attr);
    print_centered("SISTEMA OPERATIVO BASICO", attr(COLOR_CYAN, COLOR_WHITE));
    print_centered("Bootloader x86", attr(COLOR_CYAN, COLOR_WHITE));
    print_separator('=', title_attr);
    newline(attr(COLOR_BLACK, COLOR_WHITE));

    // Comprobaciones
    struct GDTR gdtr;
    bool pm = check_protected_mode();
    bool seg = check_segments_ok();
    bool gdt = check_gdt_ok(&gdtr);
    bool a20 = check_a20_enabled();
    bool vga_ok = check_vga_access();

    print_status_checked("Kernel iniciado correctamente", true);
    print_status_checked("Modo protegido activado (CR0.PE)", pm);
    print_status_checked("Segmentos CS=0x08, DS=SS=0x10", seg);
    print_status_checked("GDT cargada (base/limit validos)", gdt);
    print_status_checked("A20 habilitada (PORT 0x92h)", a20);
    print_status_checked("Acceso VGA OK (0xB8000)", vga_ok);

    newline(attr(COLOR_BLACK, COLOR_WHITE));

    // Mapa de memoria (alineado)
    print_separator('-', attr(COLOR_YELLOW, COLOR_BLACK));
    print_centered("--- MAPA DE MEMORIA ---", attr(COLOR_YELLOW, COLOR_BLACK));
    print_separator('-', attr(COLOR_YELLOW, COLOR_BLACK));

    char buf[64];
    // Stage1
    print_string("  Stage1 (bootloader): ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_hex32(0x00007C00, attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string(" - ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_hex32(0x00007DFF, attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string(" (512 bytes)", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); newline(attr(COLOR_BLACK, COLOR_LIGHT_GRAY));

    // Stage2
    print_string("  Stage2 (loader):     ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_hex32(0x00007E00, attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string(" - ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_hex32(0x00007FFF, attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string(" (1024 bytes)", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); newline(attr(COLOR_BLACK, COLOR_LIGHT_GRAY));

    // Kernel
    uint32_t kstart = (uint32_t)&_start, kend = (uint32_t)&_end;
    print_string("  Kernel:              ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_hex32(kstart, attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string(" - ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_hex32(kend, attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string(" (", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_dec_attr(kend - kstart, attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string(" bytes)", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); newline(attr(COLOR_BLACK, COLOR_LIGHT_GRAY));

    // Stack and VGA
    print_string("  Stack:               ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex32(0x00090000, attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_string(" (crece hacia abajo)", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); newline(attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string("  VGA Buffer:          ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex32(0x000B8000, attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_string(" (80x25 texto)", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); newline(attr(COLOR_BLACK, COLOR_LIGHT_GRAY));

    newline(attr(COLOR_BLACK, COLOR_WHITE));

    // Información del sistema
    print_separator('-', attr(COLOR_YELLOW, COLOR_BLACK));
    print_centered("--- INFORMACION DEL SISTEMA ---", attr(COLOR_YELLOW, COLOR_BLACK));
    print_separator('-', attr(COLOR_YELLOW, COLOR_BLACK));

    print_kv("Arquitectura:", "x86 (i386)", attr(COLOR_BLACK, COLOR_LIGHT_GRAY), attr(COLOR_BLACK, COLOR_WHITE));
    print_kv("Modo CPU:", pm ? "Protected Mode (32-bit)" : "Real Mode??", attr(COLOR_BLACK, COLOR_LIGHT_GRAY), attr(COLOR_BLACK, COLOR_WHITE));

    // Tamaño kernel y direcciones (sin snprintf)
    print_string("  Tamano kernel:       ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_dec_attr(kend - kstart, attr(COLOR_BLACK, COLOR_WHITE));
    print_string(" bytes (", attr(COLOR_BLACK, COLOR_WHITE));
    print_dec_attr(((kend - kstart)+511)/512, attr(COLOR_BLACK, COLOR_WHITE));
    print_string(" sectores)", attr(COLOR_BLACK, COLOR_WHITE)); newline(attr(COLOR_BLACK, COLOR_WHITE));

    // Direcciones
    print_string("  Direccion inicio:    ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex32(kstart, attr(COLOR_BLACK, COLOR_WHITE)); newline(attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  Direccion fin:       ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex32(kend, attr(COLOR_BLACK, COLOR_WHITE)); newline(attr(COLOR_BLACK, COLOR_WHITE));

    // GDT / ESP
    print_string("  GDT base:            ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex32(gdtr.base, attr(COLOR_BLACK, COLOR_WHITE)); newline(attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  GDT limit:           ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    // limit is 16-bit
    print_string("0x", attr(COLOR_BLACK, COLOR_WHITE));
    // print 4-digit hex for limit
    uint16_t gl = gdtr.limit;
    const char* hex = "0123456789ABCDEF";
    for (int i = 12; i >= 0; i -= 4) putchar(hex[(gl >> i) & 0xF], attr(COLOR_BLACK, COLOR_WHITE));
    newline(attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  ESP actual:          ", attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex32(read_esp(), attr(COLOR_BLACK, COLOR_WHITE)); newline(attr(COLOR_BLACK, COLOR_WHITE));

    newline(attr(COLOR_BLACK, COLOR_WHITE));

    // Pie
    print_separator('=', title_attr);
    print_centered("Sistema listo.", attr(COLOR_BLACK, COLOR_LIGHT_CYAN));
    print_separator('=', title_attr);

    while (1) { __asm__ volatile("hlt"); }
}