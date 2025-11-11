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

// Acceso al cursor
void get_cursor(int* x, int* y) { if (x) *x = cursor_x; if (y) *y = cursor_y; }
void set_cursor(int x, int y) { if (x>=0 && x<VGA_WIDTH) cursor_x = x; if (y>=0 && y<VGA_HEIGHT) cursor_y = y; }

void print_separator(char ch, unsigned char a) {
    for (int i = 0; i < VGA_WIDTH; ++i) putchar(ch, a);
    newline(a);
}

void print_centered(const char* s, unsigned char a) {
    // Calcular longitud y columna de inicio
    int len = 0; for (const char* p = s; *p; ++p) ++len;
    int x = (VGA_WIDTH - len) / 2; if (x < 0) x = 0;
    // Escribir centrado en la línea actual y avanzar cursor hasta el final del texto
    int old_y = cursor_y;
    cursor_x = x; cursor_y = old_y;
    print_string(s, a);
    cursor_x = x + len; cursor_y = old_y;
    // Ahora el newline limpiará solo desde el final del texto hacia la derecha
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
    print_string("  [", gray); putchar(' ', gray);
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

// =================== FORMATEO Y ANALISIS ===================
void print_hex_bytes(const uint8_t* p, int n, unsigned char attr_hex, unsigned char attr_sep) {
    const char* hex = "0123456789ABCDEF";
    for (int i=0; i<n; ++i) {
        uint8_t b = p[i];
        char buf[3]; buf[0] = hex[(b>>4)&0xF]; buf[1] = hex[b & 0xF]; buf[2] = 0;
        print_string(buf, attr_hex);
        if (i+1<n) { putchar(' ', attr_sep); }
    }
}

bool mod0_parse(const volatile unsigned char* m, uint16_t* out_version, uint16_t* out_length, uint32_t* out_entry_off) {
    // Aceptar cabeceras con magic "MOD0" (legacy) o "MBIN" (nuevo estándar)
    bool legacy = (m[0]=='M' && m[1]=='O' && m[2]=='D' && m[3]=='0');
    bool mbin   = (m[0]=='M' && m[1]=='B' && m[2]=='I' && m[3]=='N');
    if (!(legacy || mbin)) return false;
    uint16_t ver = (uint16_t)m[4] | ((uint16_t)m[5] << 8);
    uint16_t len = (uint16_t)m[6] | ((uint16_t)m[7] << 8);
    uint32_t ent = ((uint32_t)m[8]) | ((uint32_t)m[9] << 8) | ((uint32_t)m[10] << 16) | ((uint32_t)m[11] << 24);
    if (out_version) *out_version = ver;
    if (out_length) *out_length = len;
    if (out_entry_off) *out_entry_off = ent;
    return true;
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

// =================== PROGRESS BAR ===================
void progress_bar_ms(int total_ms, const char* label) {
    clear_screen();
    uint8_t hdr = vga_attr(THEME_BG, THEME_PRIMARY);
    uint8_t muted = vga_attr(THEME_BG, THEME_MUTED);
    uint8_t ok = vga_attr(THEME_BG, THEME_OK);
    print_separator('=', hdr);
    print_centered(label ? label : "Cargando...", hdr);
    print_separator('-', muted);
    int width = VGA_WIDTH - 10; if (width < 12) width = 12;
    int inner = width - 2;
    // Dibujar contorno
    putchar('[', muted);
    int sx = cursor_x, sy = cursor_y;
    for (int i=0;i<inner;i++) putchar(' ', muted);
    putchar(']', muted);
    int delay = total_ms / (inner ? inner : 1);
    for (int i=0;i<inner;i++) {
        cursor_x = sx + i; cursor_y = sy;
        putchar('=', ok);
        for (volatile uint32_t d=0; d < (uint32_t)(delay * 1200); ++d) __asm__ volatile("nop");
    }
    newline(muted);
    print_separator('=', hdr);
}

// Barra inline: dibuja y actualiza una barra pequeña en la misma línea actual
void progress_bar_inline_ms(int total_ms, int width) {
    if (width < 10) width = 10;
    int fill = width - 2; // parte interna
    putchar('[', vga_attr(THEME_BG, THEME_MUTED));
    int sx = cursor_x; int sy = cursor_y;
    for (int i=0;i<fill;i++) putchar(' ', vga_attr(THEME_BG, THEME_MUTED));
    putchar(']', vga_attr(THEME_BG, THEME_MUTED));
    int delay = total_ms / (fill ? fill : 1);
    for (int i=0;i<fill;i++) {
        cursor_x = sx + i; cursor_y = sy;
        putchar('=', vga_attr(THEME_BG, THEME_OK));
        // aumentar factor de retardo para que sea visible en QEMU/hosts rápidos
        for (volatile uint32_t d=0; d < (uint32_t)(delay *4294967295L); ++d) __asm__ volatile("nop");
    }
    cursor_x = sx + fill + 1; cursor_y = sy; // dejar cursor tras barra
}

// Barra en dos fases: dibujar contorno y devolver coordenadas internas
void progress_bar_inline_draw(int width, int* out_x, int* out_y, int* out_fill) {
    if (width < 10) width = 10;
    int fill = width - 2;
    putchar('[', vga_attr(THEME_BG, THEME_MUTED));
    int sx = cursor_x; int sy = cursor_y;
    for (int i=0;i<fill;i++) putchar(' ', vga_attr(THEME_BG, THEME_MUTED));
    putchar(']', vga_attr(THEME_BG, THEME_MUTED));
    if (out_x) *out_x = sx; if (out_y) *out_y = sy; if (out_fill) *out_fill = fill;
}

void progress_bar_inline_animate(int total_ms, int sx, int sy, int fill) {
    if (fill <= 0) return;
    int delay = total_ms / fill;
    for (int i=0;i<fill;i++) {
        cursor_x = sx + i; cursor_y = sy;
        putchar('=', vga_attr(THEME_BG, THEME_OK));
        for (volatile uint32_t d=0; d < (uint32_t)(delay * 15000); ++d) __asm__ volatile("nop");
    }
    cursor_x = sx + fill + 1; cursor_y = sy;
}


// Leer scancode bloqueando hasta que haya dato
static inline uint8_t read_scancode_block(void) {
    for (;;) {
        if (inb(0x64) & 0x01) break;
        for (volatile uint32_t d=0; d<20000; ++d) __asm__ volatile("nop");
    }
    return (uint8_t)inb(0x60);
}

// Vacía scancodes pendientes (manejando prefijo 0xE0)
void kbd_flush(void) {
    for (int i=0; i<64; ++i) {
        if (!(inb(0x64) & 0x01)) break;
        uint8_t sc = (uint8_t)inb(0x60);
        if (sc == 0xE0) {
            for (volatile uint32_t d=0; d<8000; ++d) __asm__ volatile("nop");
            if (inb(0x64) & 0x01) (void)inb(0x60);
        }
        for (volatile uint32_t d=0; d<8000; ++d) __asm__ volatile("nop");
    }
}

// Espera a una PULSACIÓN válida (solo make codes) pero exige RELEASE si la tecla ya estaba mantenida
bool wait_for_keypress(void) {
    kbd_flush();

    // Detectar si hay una tecla actualmente "mantenida" (makes repetidos) en una ventana corta
    bool held = false;
    for (int i=0;i<20;i++) {
        if (inb(0x64) & 0x01) {
            uint8_t s = (uint8_t)inb(0x60);
            if (s == 0xE0) { if (inb(0x64) & 0x01) (void)inb(0x60); }
            if ((s & 0x80) == 0) { held = true; break; } // detectado make -> tecla sostenida
        }
        for (volatile uint32_t d=0; d<12000; ++d) __asm__ volatile("nop");
    }

    if (held) {
        // esperar el break (release) de cualquier tecla antes de aceptar nueva pulsación
        for (;;) {
            uint8_t sc = read_scancode_block();
            if (sc == 0xE0) {
                uint8_t sc2 = read_scancode_block();
                if (sc2 & 0x80) break; // break extendido
                else continue; // ignore extra make
            } else {
                if (sc & 0x80) break; // break normal
                else continue; // ignore make repeats
            }
        }
        // al salir del bucle hemos visto una release; ahora seguimos a esperar la siguiente make
    }

    // Esperar el make code (nueva pulsación)
    for (;;) {
        uint8_t sc = read_scancode_block();
        if (sc == 0xE0) {
            uint8_t sc2 = read_scancode_block();
            if ((sc2 & 0x80) == 0) return true; // make extendido
            else continue;
        } else {
            if ((sc & 0x80) == 0) return true; // make normal
            else continue;
        }
    }
}

// Vacía cualquier scancode pendiente antes de una espera activa


// Timeout genérico reutilizado por kernel.c (si no existe tecla simplemente expira)
bool wait_key_or_timeout_ms(int timeout_ms) {
    int slices = timeout_ms / 10; if (slices <= 0) slices = 1;
    for (int i=0; i<slices; ++i) {
        if (inb(0x64) & 0x01) { (void)inb(0x60); return true; }
        for (volatile uint32_t d=0; d<30000; ++d) __asm__ volatile("nop");
    }
    return false;
}

// =================== ATA PIO (28-bit LBA, 1 sector) ===================
static inline void outb(uint16_t port, uint8_t val){ __asm__ volatile("outb %0, %1"::"a"(val),"Nd"(port)); }
static inline uint8_t inb_p(uint16_t port){ uint8_t v; __asm__ volatile("inb %1, %0":"=a"(v):"Nd"(port)); return v; }
static int ata_wait_bsy_drq(){
    for (int i=0;i<100000;i++) { uint8_t s=inb_p(0x1F7); if(!(s&0x80) && (s&0x08)) return 0; }
    return -1;
}

int ata_read28(uint32_t lba, void* dst) {
    // Seleccionar disco primario maestro con LBA alto
    outb(0x1F6, 0xE0 | ((lba>>24)&0x0F));
    outb(0x1F2, 1);                          // 1 sector
    outb(0x1F3, (uint8_t)(lba & 0xFF));
    outb(0x1F4, (uint8_t)((lba>>8) & 0xFF));
    outb(0x1F5, (uint8_t)((lba>>16)& 0xFF));
    outb(0x1F7, 0x20);                        // READ SECTORS
    if (ata_wait_bsy_drq()!=0) return -1;
    uint16_t* wdst = (uint16_t*)dst;
    int count = 256;
    __asm__ volatile ("rep insw" : "+D"(wdst), "+c"(count) : "d"(0x1F0) : "memory");
    return 0;
}

bool load_kernel_post(void) {
    // LBA del kernel en la imagen: 3 (después de stage1 y stage2)
    // IMPORTANTE: usar el recuento real de sectores cargados por stage2 (excluye .bss),
    // ya que &_end - &_start incluye .bss y puede desincronizarse con el tamaño en disco.
    volatile uint8_t* status = (volatile uint8_t*)STAGE2_BASE;
    uint32_t ksects = *(volatile uint8_t*)STAGE2_KERNEL_SECTS; // KERNEL_SECTS establecido por stage2.asm (header)
    uint32_t module_lba = 3 + ksects;
    if (ata_read28(module_lba, (void*)MODULE_LOAD_ADDRESS) == 0) {
        // Validar magic aceptado
        volatile char* m = (volatile char*)MODULE_LOAD_ADDRESS;
        bool ok = ((m[0]=='M' && m[1]=='O' && m[2]=='D' && m[3]=='0') || (m[0]=='M' && m[1]=='B' && m[2]=='I' && m[3]=='N'));
        if (ok) {
            *(volatile uint8_t*)STAGE2_MODULE_OK = 1; // MODULE_OK
            *(volatile uint8_t*)STAGE2_MODULE_SECTS = 1; // MODULE_SECTS (1 sector)
            return true;
        } else {
            // No hay módulo válido
            *(volatile uint8_t*)STAGE2_MODULE_OK = 0; *(volatile uint8_t*)STAGE2_MODULE_SECTS = 0;
            return false;
        }
    }
    return false;
}

// ======= Sonda y finalización de módulo para UI =======
static uint8_t g_module_buf[512];

bool module_probe(void) {
    // Calcular LBA del módulo igual que load_kernel_post, pero sin efectos secundarios.
    // Usar KERNEL_SECTS reportado por stage2 para evitar incluir .bss en el cómputo.
    volatile uint8_t* status = (volatile uint8_t*)STAGE2_BASE;
    uint32_t ksects = *(volatile uint8_t*)STAGE2_KERNEL_SECTS;
    uint32_t module_lba = 3 + ksects;
    if (ata_read28(module_lba, (void*)g_module_buf) != 0) return false;
    bool legacy = (g_module_buf[0]=='M' && g_module_buf[1]=='O' && g_module_buf[2]=='D' && g_module_buf[3]=='0');
    bool mbin   = (g_module_buf[0]=='M' && g_module_buf[1]=='B' && g_module_buf[2]=='I' && g_module_buf[3]=='N');
    return legacy || mbin;
}

bool module_finalize_after_bar(void) {
    // Copiar buffer sondeado a la dirección final y, si el header indica más tamaño,
    // leer sectores adicionales desde disco.
    uint8_t* dst = (uint8_t*)MODULE_LOAD_ADDRESS;
    for (int i = 0; i < 512; ++i) dst[i] = g_module_buf[i];

    // Parsear cabecera para conocer longitud total
    uint16_t ver=0, length_field=0; uint32_t entry_off=0;
    bool header_ok = mod0_parse((const volatile unsigned char*)dst, &ver, &length_field, &entry_off);
    // Longitud mínima segura: al menos 512 si no hay header válido
    uint32_t total_len = header_ok ? (uint32_t)length_field : 512u;
    if (total_len < 512u) total_len = 512u; // redondeo defensivo
    uint32_t sectors_needed = (total_len + 511u) / 512u;

    // Calcular LBA base del módulo
    volatile uint8_t* status = (volatile uint8_t*)STAGE2_BASE;
    uint32_t ksects = *(volatile uint8_t*)STAGE2_KERNEL_SECTS;
    uint32_t module_lba = 3 + ksects;

    // Leer los sectores restantes (ya tenemos el primero)
    uint32_t loaded = 1;
    for (uint32_t s = 1; s < sectors_needed; ++s) {
        if (ata_read28(module_lba + s, (void*)(MODULE_LOAD_ADDRESS + s*512)) != 0) {
            break; // parar ante fallo de E/S
        }
        loaded++;
    }
    *(volatile uint8_t*)STAGE2_MODULE_OK = (loaded==sectors_needed) ? 1 : 1; // marcar como presente aunque incompleto
    *(volatile uint8_t*)STAGE2_MODULE_SECTS = (uint8_t)loaded;
    return loaded >= 1;
}
