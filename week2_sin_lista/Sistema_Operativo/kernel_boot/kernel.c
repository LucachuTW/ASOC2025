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
extern bool check_gdt_ok(struct GDTR *out);
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

	// Sonda rápida del módulo, solo el contorno porque el relleno será en la próxima ronda de dibujado
	bool module_present = module_probe();
	void (*module_entry_fn)(void) = 0; // diferir ejecución hasta el final de la página
	int bar_x = 0, bar_y = 0, bar_fill = 0;
	// Sección módulo: dirección y estado de sondeo
	print_string("  Modulo opcional: ", C(THEME_MUTED));
	print_hex(MODULE_LOAD_ADDRESS);
	print_string(" ", C(THEME_MUTED));
	if (module_present) {
		print_string("SONDEADO", vga_attr(THEME_BG, THEME_OK));
		print_string(" ", C(THEME_MUTED));
		progress_bar_inline_draw(28, &bar_x, &bar_y, &bar_fill);
		print_string(" listo", C(THEME_MUTED));
	} else {
		print_string("NO", vga_attr(THEME_BG, THEME_ERR));
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

	print_separator(' ', sep2);
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
	print_string("  PseudoKernel:  ", C(THEME_MUTED));
	print_hex(kstart);
	print_string(" - ", C(THEME_MUTED));
	print_hex(kend);
	print_string(" (", C(THEME_MUTED));
	print_dec(ksize);
	print_string(" bytes)", C(THEME_MUTED));
	putchar('\n', C(THEME_MUTED));

	/* Mostrar ESP real en lugar de una constante hardcodeada */
	print_string("  Stack (ESP):   ", C(THEME_MUTED));
	uint32_t esp_now = read_esp();
	print_hex(esp_now);
	putchar('\n', C(THEME_MUTED));

	/* Mostrar la dirección VGA usando la macro del kernel */
	print_string("  VGA:           ", C(THEME_MUTED));
	print_hex((uint32_t)VGA_ADDRESS);
	putchar('\n', C(THEME_MUTED));

	newline(C(THEME_TEXT));
	print_separator('=', sep1);

	if (!module_present) {
		print_centered("Sistema listo", vga_attr(THEME_BG, THEME_PRIMARY));
		print_separator('=', sep1);
	}

	else {
		// Ahora que toda la pantalla está dibujada, animamos la barra en sitio
		// Animar barra y finalizar copia del módulo
		progress_bar_inline_animate(2500, bar_x, bar_y, bar_fill);
		bool finalized = module_finalize_after_bar();
		print_string("  Copia modulo: ", C(THEME_MUTED));
		print_string(finalized ? "OK" : "FALLO", finalized ? vga_attr(THEME_BG, THEME_OK) : vga_attr(THEME_BG, THEME_ERR));
		newline(C(THEME_TEXT));
		// Título compacto detalles de módulo (reducimos altura)
		print_string("  -- Detalles modulo --", vga_attr(THEME_BG, THEME_ACCENT));
		newline(C(THEME_TEXT));
		// Mostrar OK a continuación de la barra
		set_cursor(bar_x + bar_fill + 1, bar_y); // ya queda ahí tras animate, reforzamos
		print_string(" OK", vga_attr(THEME_BG, THEME_OK));
		newline(C(THEME_TEXT));
		// Resolver y validar cabecera del módulo antes de ejecutar
		volatile unsigned char *m = (volatile unsigned char *)MODULE_LOAD_ADDRESS;
		uint16_t version = 0, length_field = 0;
		uint32_t entry_off = 0;
		bool header_ok = mod0_parse(m, &version, &length_field, &entry_off);
		print_string("  Cabecera: ", C(THEME_MUTED));
		if (header_ok) {
			print_string("OK", vga_attr(THEME_BG, THEME_OK));
			// Validar rango de entry
			bool entry_valid = (version == 1 && entry_off >= 16 && entry_off < 512 && (uintptr_t)MODULE_LOAD_ADDRESS + entry_off < (uintptr_t)MODULE_LOAD_ADDRESS + 512);
			if (entry_valid)
				module_entry_fn = (void (*)(void))((uintptr_t)MODULE_LOAD_ADDRESS + entry_off);
			print_string(" ver=", C(THEME_MUTED));
			print_dec(version);
			print_string(" len=", C(THEME_MUTED));
			print_dec(length_field);
			print_string(" entry_off=", C(THEME_MUTED));
			print_hex(entry_off);
			print_string(entry_valid ? " (valido)" : " (invalido)", entry_valid ? vga_attr(THEME_BG, THEME_OK) : vga_attr(THEME_BG, THEME_ERR));
		} else {
			print_string("NO HEADER", vga_attr(THEME_BG, THEME_ERR));
		}
		newline(C(THEME_TEXT));
		print_string("  Dump[0..15]: ", C(THEME_MUTED));
		print_hex_bytes((const uint8_t *)m, 16, C(THEME_TEXT), C(THEME_MUTED));
		newline(C(THEME_TEXT));
		// Espera explícita del usuario para ejecutar el módulo
		print_centered("Pulsa cualquier tecla para ejecutar el modulo", vga_attr(THEME_BG, THEME_ACCENT));
		kbd_flush();
		wait_for_keypress();
		// Ejecutar el módulo (dejamos la pantalla previa visible hasta que el propio módulo limpie)
		if (module_entry_fn) {
			// Validación adicional para evitar crash si puntero sale del rango del sector
			uintptr_t me_addr = (uintptr_t)module_entry_fn;
			bool inside_sector = me_addr >= (uintptr_t)MODULE_LOAD_ADDRESS && me_addr < (uintptr_t)MODULE_LOAD_ADDRESS + 512;
			if (!inside_sector) {
				print_string("  Entry fuera de rango (>=512b): abort", vga_attr(THEME_BG, THEME_ERR));
				newline(C(THEME_TEXT));
			} else {
				print_string("  Entry OK -> ", C(THEME_MUTED));
				print_hex((uint32_t)(uintptr_t)module_entry_fn);
				newline(C(THEME_TEXT));
				uint32_t esp_before = read_esp();
				print_string("  ESP antes: ", C(THEME_MUTED));
				print_hex(esp_before);
				newline(C(THEME_TEXT));
				module_entry_fn();
				uint32_t esp_after = read_esp();
				print_string("  ESP despues: ", C(THEME_MUTED));
				print_hex(esp_after);
				newline(C(THEME_TEXT));
				print_string("  Retorno modulo", vga_attr(THEME_BG, THEME_OK));
				newline(C(THEME_TEXT));
			}
		} else {
			print_string("  Entry invalido: no se ejecuta modulo", vga_attr(THEME_BG, THEME_ERR));
			newline(C(THEME_TEXT));
		}
		// Espera breve para que puedas leer la salida del módulo
		print_centered("[Modulo finalizado - 3s]", vga_attr(THEME_BG, THEME_OK));
		wait_key_or_timeout_ms(3000);
		// Redibujar pantalla completa del pseudokernel con mensaje final
		clear_screen();
		print_centered("PseudoKernel", vga_attr(THEME_BG, THEME_TEXT));
		print_centered("Virus Payal OS Bootloader by Emefedez", vga_attr(THEME_BG, THEME_PRIMARY));
		print_separator('=', sep1);
		// Línea de módulo (ya cargado)
		print_string("  Modulo opcional:     ", C(THEME_MUTED));
		print_hex(MODULE_LOAD_ADDRESS);
		print_string(" CARGADO", vga_attr(THEME_BG, THEME_OK));
		newline(C(THEME_TEXT));
		newline(C(THEME_TEXT));
		// Comprobaciones de nuevo
		struct GDTR gdtr2;
		bool pm2 = check_protected_mode();
		bool seg2 = check_segments_ok();
		bool gdt2 = check_gdt_ok(&gdtr2);
		bool a202 = check_a20_enabled();
		bool vga2 = check_vga_access();
		print_separator('-', sep2);
		print_status_checked("Modo protegido activado", pm2);
		print_status_checked("Segmentos configurados", seg2);
		print_status_checked("GDT cargada", gdt2);
		print_status_checked("A20 habilitada", a202);
		print_status_checked("Acceso VGA", vga2);
		newline(C(THEME_TEXT));
		print_separator('-', vga_attr(THEME_BG, THEME_ACCENT));
	uint32_t kstart2 = (uint32_t)&_start, kend2 = (uint32_t)&_end;
	uint32_t ksize2 = kend2 - kstart2;
	print_string("  PseudoKernel:  ", C(THEME_MUTED));
	print_hex(kstart2);
	print_string(" - ", C(THEME_MUTED));
	print_hex(kend2);
	print_string(" (", C(THEME_MUTED));
	print_dec(ksize2);
	print_string(" bytes)", C(THEME_MUTED));
	putchar('\n', C(THEME_MUTED));

	/* Mostrar ESP real y dirección VGA (no hardcodeado) */
	print_string("  Stack (ESP):   ", C(THEME_MUTED));
	print_hex(read_esp());
	putchar('\n', C(THEME_MUTED));
	print_string("  VGA:           ", C(THEME_MUTED));
	print_hex((uint32_t)VGA_ADDRESS);
	putchar('\n', C(THEME_MUTED));
		newline(C(THEME_TEXT));
		print_separator('=', sep1);
		print_centered("Ejecucion exitosa", vga_attr(THEME_BG, THEME_OK));
		print_separator('=', sep1);
	}

	// Evitar hlt para no depender de interrupciones; bucle inofensivo
	while (1) {
		__asm__ volatile("nop");
	}
}
