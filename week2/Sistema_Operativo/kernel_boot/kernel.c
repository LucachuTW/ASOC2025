// Pseudokernel reorganizado: ahora en kernel_boot/
#include "kernel.h"

extern uint32_t _start; // inicio del kernel
extern uint32_t _end;   // fin del kernel
extern uint32_t kernel_checksum(void);
extern bool detect_optional_module(void);
extern void show_module_status(void);
extern void countdown(int seconds);
extern bool check_protected_mode(void);
extern bool check_segments_ok(void);
extern bool check_gdt_ok(struct GDTR* out);
extern bool check_a20_enabled(void);
extern bool check_vga_access(void);
extern uint32_t read_esp(void);

static uint8_t C(uint8_t fg) { return vga_attr(THEME_BG, fg); }

void kmain(void) {
    clear_screen();

    countdown(3);

    bool module_present = detect_optional_module();
    if (!module_present) {
        clear_screen();
        unsigned char a = vga_attr(THEME_BG, THEME_ERR);
        print_separator('X', a);
        print_centered("MODULO OPCIONAL NO ENCONTRADO", a);
        print_centered("Sistema detenido.", a);
        print_separator('X', a);
        while (1) { __asm__ volatile("hlt"); }
    }

    // Banner principal
    uint8_t sep1 = vga_attr(THEME_BG, THEME_SECONDARY);
    uint8_t sep2 = vga_attr(THEME_BG, THEME_PRIMARY);
    print_separator('=', sep1);
    print_centered("SISTEMA OPERATIVO BASICO", vga_attr(THEME_BG, THEME_SECONDARY));
    print_centered("Bootloader x86 - PSEUDOKERNEL", vga_attr(THEME_BG, THEME_PRIMARY));
    print_separator('=', sep2);
    newline(C(THEME_TEXT));

    // Estado de carga
    show_module_status();
    newline(C(THEME_TEXT));

    // Comprobaciones runtime
    struct GDTR gdtr;
    bool pm = check_protected_mode();
    bool seg = check_segments_ok();
    bool gdt = check_gdt_ok(&gdtr);
    bool a20 = check_a20_enabled();
    bool vga_ok = check_vga_access();
    uint32_t cksum = kernel_checksum();

    print_status_checked("Kernel iniciado correctamente", true);
    print_status_checked("Modo protegido activado (CR0.PE)", pm);
    print_status_checked("Segmentos CS=0x08, DS=SS=0x10", seg);
    print_status_checked("GDT cargada (base/limit validos)", gdt);
    print_status_checked("A20 habilitada (PORT 0x92h)", a20);
    print_status_checked("Acceso VGA OK (0xB8000)", vga_ok);

    newline(C(THEME_TEXT));

    // Mapa de memoria
    print_separator('-', vga_attr(THEME_BG, THEME_ACCENT));
    print_centered("MAPA DE MEMORIA", vga_attr(THEME_BG, THEME_ACCENT));
    print_separator('-', vga_attr(THEME_BG, THEME_ACCENT));

    uint32_t kstart = (uint32_t)&_start, kend = (uint32_t)&_end;
    uint32_t ksize = kend - kstart;
    print_string("  Stage1 (bootloader): ", C(THEME_MUTED)); print_hex(0x00007C00); print_string(" - ", C(THEME_MUTED)); print_hex(0x00007DFF); print_string(" (512 bytes)", C(THEME_MUTED)); putchar('\n', C(THEME_MUTED));
    print_string("  Stage2 (loader):     ", C(THEME_MUTED)); print_hex(0x00007E00); print_string(" - ", C(THEME_MUTED)); print_hex(0x00007FFF); print_string(" (1024 bytes)", C(THEME_MUTED)); putchar('\n', C(THEME_MUTED));
    print_string("  Kernel:              ", C(THEME_MUTED)); print_hex(kstart); print_string(" - ", C(THEME_MUTED)); print_hex(kend); print_string(" (", C(THEME_MUTED)); print_dec(ksize); print_string(" bytes)", C(THEME_MUTED)); putchar('\n', C(THEME_MUTED));
    print_string("  Modulo opcional:     ", C(THEME_MUTED)); print_hex(MODULE_LOAD_ADDRESS); print_string(" (si presente)", C(THEME_MUTED)); putchar('\n', C(THEME_MUTED));
    print_string("  Stack:               ", C(THEME_MUTED)); print_hex(0x00090000); print_string(" (crece hacia abajo)", C(THEME_MUTED)); putchar('\n', C(THEME_MUTED));
    print_string("  VGA Buffer:          ", C(THEME_MUTED)); print_hex(0x000B8000); print_string(" (80x25 texto)", C(THEME_MUTED)); putchar('\n', C(THEME_MUTED));

    newline(C(THEME_TEXT));

    // Información del sistema
    print_separator('-', vga_attr(THEME_BG, THEME_PRIMARY));
    print_centered("INFORMACION DEL SISTEMA", vga_attr(THEME_BG, THEME_PRIMARY));
    print_separator('-', vga_attr(THEME_BG, THEME_PRIMARY));
    print_kv("Arquitectura:", "x86 (i386)", C(THEME_MUTED), C(THEME_TEXT));
    print_kv("Modo CPU:", pm ? "Protected Mode (32-bit)" : "Real Mode??", C(THEME_MUTED), C(THEME_TEXT));
    print_string("  Tamano kernel:       ", C(THEME_MUTED)); print_dec(ksize); print_string(" bytes (", C(THEME_TEXT)); print_dec(((ksize)+511)/512); print_string(" sectores)", C(THEME_TEXT)); putchar('\n', C(THEME_TEXT));
    print_string("  Direccion inicio:    ", C(THEME_MUTED)); print_hex(kstart); putchar('\n', C(THEME_TEXT));
    print_string("  Direccion fin:       ", C(THEME_MUTED)); print_hex(kend); putchar('\n', C(THEME_TEXT));
    print_string("  Checksum kernel:     ", C(THEME_MUTED)); print_hex(cksum); putchar('\n', C(THEME_TEXT));
    print_string("  GDT base:            ", C(THEME_MUTED)); print_hex(gdtr.base); putchar('\n', C(THEME_TEXT));
    print_string("  GDT limit:           ", C(THEME_MUTED)); print_hex(gdtr.limit); putchar('\n', C(THEME_TEXT));
    print_string("  ESP actual:          ", C(THEME_MUTED)); print_hex(read_esp()); putchar('\n', C(THEME_TEXT));

    newline(C(THEME_TEXT));
    print_separator('=', sep2);
    print_centered("Sistema listo. Modulo preparado.", vga_attr(THEME_BG, THEME_OK));
    print_separator('=', sep2);

    while (1) { __asm__ volatile("hlt"); }
}
