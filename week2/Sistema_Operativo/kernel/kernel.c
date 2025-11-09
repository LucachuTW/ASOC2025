#include "kernel.h"

static uint16_t* vga = (uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;

void clear_screen(void) {
	for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
		vga[i] = (COLOR_BLACK << 8) | ' ';
	}
	cursor_x = 0;
	cursor_y = 0;
}

void putchar(char c, uint8_t color) {
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else {
		int offset = cursor_y * VGA_WIDTH + cursor_x;
		vga[offset] = (color << 8) | c;
		cursor_x++;
		if (cursor_x >= VGA_WIDTH) {
			cursor_x = 0;
			cursor_y++;
		}
	}
	
	// Scroll simple
	if (cursor_y >= VGA_HEIGHT) {
		cursor_y = VGA_HEIGHT - 1;
		// Mover líneas hacia arriba
		for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
			vga[i] = vga[i + VGA_WIDTH];
		}
		// Limpiar última línea
		for (int i = 0; i < VGA_WIDTH; i++) {
			vga[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = (COLOR_BLACK << 8) | ' ';
		}
	}
}

void print_string(const char* str, uint8_t color) {
	while (*str) {
		putchar(*str++, color);
	}
}

void print_hex(uint32_t value) {
	char hex[] = "0x00000000";
	for (int i = 9; i >= 2; i--) {
		uint8_t nibble = value & 0xF;
		hex[i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
		value >>= 4;
	}
	print_string(hex, COLOR_WHITE);
}

void kmain(void) {
	clear_screen();
	
	print_string("[ Virus Payal OS ]\n", COLOR_GREEN);
	print_string("================\n\n", COLOR_WHITE);
	
	print_string("Kernel cargado en: ", COLOR_WHITE);
	print_hex(0x8000);
	print_string("\n", COLOR_WHITE);
	
	print_string("Estado: ", COLOR_WHITE);
	print_string("OK\n", COLOR_GREEN);
	
	print_string("\nSistema iniciado correctamente.\n", COLOR_WHITE);
	
	// Bucle infinito
	for(;;);
}