// Pseudokernel: kmain y lógica de alto nivel (helpers movidos a kernelfunciones.c)
#include "kernel.h"

// =================== ENLACES A SIMBOLOS DEL LINKER ===================
extern uint32_t _start; // inicio del kernel
extern uint32_t _end;   // fin del kernel
// Helpers están en kernelfunciones.c
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
// (Cuenta atrás implementada en kernelfunciones.c, restos limpiados)

// =================== FUNCIÓN PRINCIPAL ===================
void kmain(void) {
    clear_screen();

    countdown(3);

    bool module_present = detect_optional_module();
    if (!module_present) {
        clear_screen();
        unsigned char a = vga_attr(COLOR_BLUE, COLOR_LIGHT_RED);
        print_separator('X', a);
        print_centered("MODULO OPCIONAL NO ENCONTRADO", a);
        print_centered("Sistema detenido.", a);
        print_separator('X', a);
        while (1) { __asm__ volatile("hlt"); }
    }

    // Banner vibrante
    uint8_t title1 = vga_attr(COLOR_LIGHT_MAGENTA, COLOR_BLACK);
    uint8_t title2 = vga_attr(COLOR_LIGHT_CYAN, COLOR_BLACK);
    print_separator('=', title1);
    print_centered("SISTEMA OPERATIVO BASICO", vga_attr(COLOR_LIGHT_MAGENTA, COLOR_WHITE));
    print_centered("Bootloader x86 - PSEUDOKERNEL", vga_attr(COLOR_LIGHT_CYAN, COLOR_WHITE));
    print_separator('=', title2);
    newline(vga_attr(COLOR_BLACK, COLOR_WHITE));

    // Estado de carga
    show_module_status();
    newline(vga_attr(COLOR_BLACK, COLOR_WHITE));

    // Comprobaciones
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

    newline(vga_attr(COLOR_BLACK, COLOR_WHITE));

    // Mapa de memoria
    print_separator('-', vga_attr(COLOR_YELLOW, COLOR_BLACK));
    print_centered("MAPA DE MEMORIA", vga_attr(COLOR_YELLOW, COLOR_WHITE));
    print_separator('-', vga_attr(COLOR_YELLOW, COLOR_BLACK));

    uint32_t kstart = (uint32_t)&_start, kend = (uint32_t)&_end;
    print_string("  Stage1 (bootloader): ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(0x00007C00); print_string(" - ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(0x00007DFF); print_string(" (512 bytes)", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); putchar('\n', vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string("  Stage2 (loader):     ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(0x00007E00); print_string(" - ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(0x00007FFF); print_string(" (1024 bytes)", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); putchar('\n', vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string("  Kernel:              ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(kstart); print_string(" - ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(kend); print_string(" (", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_dec(kend - kstart); print_string(" bytes)", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); putchar('\n', vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string("  Modulo opcional:     ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(MODULE_LOAD_ADDRESS); print_string(" (si presente)", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); putchar('\n', vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string("  Stack:               ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(0x00090000); print_string(" (crece hacia abajo)", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); putchar('\n', vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY));
    print_string("  VGA Buffer:          ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(0x000B8000); print_string(" (80x25 texto)", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); putchar('\n', vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY));

    newline(vga_attr(COLOR_BLACK, COLOR_WHITE));

    // Info sistema
    print_separator('-', vga_attr(COLOR_LIGHT_BLUE, COLOR_BLACK));
    print_centered("INFORMACION DEL SISTEMA", vga_attr(COLOR_LIGHT_BLUE, COLOR_WHITE));
    print_separator('-', vga_attr(COLOR_LIGHT_BLUE, COLOR_BLACK));
    print_kv("Arquitectura:", "x86 (i386)", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY), vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_kv("Modo CPU:", pm ? "Protected Mode (32-bit)" : "Real Mode??", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY), vga_attr(COLOR_BLACK, COLOR_WHITE));
    // Tamaño kernel
    // Línea placeholder eliminada
    print_string("  Tamano kernel:       ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_dec(kend - kstart); print_string(" bytes (", vga_attr(COLOR_BLACK, COLOR_WHITE)); print_dec(((kend - kstart)+511)/512); print_string(" sectores)", vga_attr(COLOR_BLACK, COLOR_WHITE)); putchar('\n', vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  Direccion inicio:    ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(kstart); putchar('\n', vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  Direccion fin:       ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(kend); putchar('\n', vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  Checksum kernel:     ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(cksum); putchar('\n', vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  GDT base:            ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(gdtr.base); putchar('\n', vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  GDT limit:           ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(gdtr.limit); putchar('\n', vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_string("  ESP actual:          ", vga_attr(COLOR_BLACK, COLOR_LIGHT_GRAY)); print_hex(read_esp()); putchar('\n', vga_attr(COLOR_BLACK, COLOR_WHITE));

    newline(vga_attr(COLOR_BLACK, COLOR_WHITE));
    print_separator('=', title2);
    print_centered("Sistema listo. Modulo preparado.", vga_attr(COLOR_BLACK, COLOR_LIGHT_GREEN));
    print_separator('=', title2);

    while (1) { __asm__ volatile("hlt"); }
}