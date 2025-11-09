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

    // Banner principal
    uint8_t sep1 = vga_attr(THEME_BG, THEME_SECONDARY);
    uint8_t sep2 = vga_attr(THEME_BG, THEME_PRIMARY);
    print_centered("PseudoKernel", vga_attr(THEME_BG, THEME_TEXT));
	print_centered("Virus Payal OS Bootloader by Emefedez",
	 vga_attr(THEME_BG, THEME_PRIMARY));
    print_separator('=', sep1);

    // Sonda rápida del módulo: solo mostrar barra si existe
    bool module_present = module_probe();
    print_string("  Modulo opcional:     ", C(THEME_MUTED)); 
    print_hex(MODULE_LOAD_ADDRESS); 
    print_string(" ", C(THEME_MUTED));
    if (module_present) {
        progress_bar_inline_ms(2000, 30);
        // Cargar definitivamente el módulo tras la barra
        module_finalize_after_bar();
        print_string(" OK", vga_attr(THEME_BG, THEME_OK));
    } else {
        print_string(" NO", vga_attr(THEME_BG, THEME_ERR));
    }
    newline(C(THEME_TEXT));

    newline(C(THEME_TEXT));

    // Comprobaciones runtime
    struct GDTR gdtr;
    bool pm = check_protected_mode();
    bool seg = check_segments_ok();
    bool gdt = check_gdt_ok(&gdtr);
    bool a20 = check_a20_enabled();
    bool vga_ok = check_vga_access();

    print_separator('-', sep2);
    print_status_checked("Modo protegido activado", pm);
    print_status_checked("Segmentos configurados", seg);
    print_status_checked("GDT cargada", gdt);
    print_status_checked("A20 habilitada", a20);
    print_status_checked("Acceso VGA", vga_ok);

    newline(C(THEME_TEXT));

    // Mapa de memoria simplificado
    print_separator('-', vga_attr(THEME_BG, THEME_ACCENT));
    uint32_t kstart = (uint32_t)&_start, kend = (uint32_t)&_end;
    uint32_t ksize = kend - kstart;
    print_string("  Kernel:  ", C(THEME_MUTED)); print_hex(kstart); print_string(" - ", C(THEME_MUTED)); print_hex(kend); print_string(" (", C(THEME_MUTED)); print_dec(ksize); print_string(" bytes)", C(THEME_MUTED)); putchar('\n', C(THEME_MUTED));
    print_string("  Stack:   ", C(THEME_MUTED)); print_hex(0x00090000); putchar('\n', C(THEME_MUTED));
    print_string("  VGA:     ", C(THEME_MUTED)); print_hex(0x000B8000); putchar('\n', C(THEME_MUTED));

    newline(C(THEME_TEXT));
    print_separator('=', sep1);
    if (module_present) {
        print_centered("Sistema listo - Modulo cargado", vga_attr(THEME_BG, THEME_OK));

    } else {
        print_centered("Sistema listo", vga_attr(THEME_BG, THEME_PRIMARY));

    }
    print_separator('=', sep1);

    while (1) { __asm__ volatile("hlt"); }
}
