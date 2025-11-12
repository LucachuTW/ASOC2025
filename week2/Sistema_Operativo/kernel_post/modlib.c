#include "modlib.h"
#include <stdbool.h>

void cls(uint8_t attr){
    uint16_t fill = ((uint16_t)attr<<8) | ' ';
    for(int i=0;i<VGA_W*VGA_H;i++) VGA_MEM[i]=fill;
}

void putc_xy(char c,int x,int y,uint8_t attr){
    if(x<0||x>=VGA_W||y<0||y>=VGA_H) return;
    VGA_MEM[y*VGA_W + x] = ((uint16_t)attr<<8) | (uint8_t)c;
}

void print_xy(const char* s,int x,int y,uint8_t attr){
    int cx=x; for(;*s; ++s,++cx) putc_xy(*s,cx,y,attr);
}

void delay(volatile uint32_t loops){ while(loops--) __asm__ volatile("nop"); }

uint8_t inb(uint16_t port){ uint8_t v; __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port)); return v; }

// Cambiado: key_pressed solo consulta el estado (NO CONSUME el scancode).
// Esto permite distinguir entre scancodes pendientes y nuevas pulsaciones.
int key_pressed(void){
    return (inb(0x64) & 0x01) ? 1 : 0;
}

void fmt_hex(uint32_t v, char* out){
    static const char* hx = "0123456789ABCDEF";
    for(int i=7; i>=0; --i){ out[i] = hx[v & 0xF]; v >>= 4; }
    out[8] = 0;
}

void u32_to_dec(uint32_t v, char* out){
    if(v==0){ out[0]='0'; out[1]=0; return; }
    char tmp[10]; int n=0; while(v && n<10){ tmp[n++] = (char)('0'+(v%10)); v/=10; }
    int i=0; while(n--) out[i++] = tmp[n]; out[i]=0;
}

void print_label_hex(const char* label, uint32_t value, int y, uint8_t la, uint8_t va){
    char h[9]; fmt_hex(value,h); print_xy(label,2,y,la); print_xy(h,14,y,va);
}

// leer scancode bloqueando hasta que haya dato
static inline uint8_t mod_read_scancode_block(void) {
    for (;;) {
        if (inb(0x64) & 0x01) break;
        for (volatile uint32_t d=0; d<20000; ++d) __asm__ volatile("nop");
    }
    return (uint8_t)inb(0x60);
}

// Vacía scancodes pendientes (manejando 0xE0)
void mod_kbd_flush(void){
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

// wait_for_keypress: exige release si tecla estaba sostenida, luego acepta el siguiente make
void mod_wait_for_keypress(void){
    mod_kbd_flush();

    // comprobar si hay makes repetidos -> tecla mantenida
    bool held = false;
    for (int i=0;i<20;i++) {
        if (inb(0x64) & 0x01) {
            uint8_t s = (uint8_t)inb(0x60);
            if (s == 0xE0) { if (inb(0x64) & 0x01) (void)inb(0x60); }
            if ((s & 0x80) == 0) { held = true; break; }
        }
        for (volatile uint32_t d=0; d<12000; ++d) __asm__ volatile("nop");
    }

    if (held) {
        // esperar release (break) antes de aceptar nueva pulsación
        for (;;) {
            uint8_t sc = mod_read_scancode_block();
            if (sc == 0xE0) {
                uint8_t sc2 = mod_read_scancode_block();
                if (sc2 & 0x80) break;
                else continue;
            } else {
                if (sc & 0x80) break;
                else continue;
            }
        }
    }

    // ahora esperar make válido
    for (;;) {
        uint8_t sc = mod_read_scancode_block();
        if (sc == 0xE0) {
            uint8_t sc2 = mod_read_scancode_block();
            if ((sc2 & 0x80) == 0) return;
            else continue;
        } else {
            if ((sc & 0x80) == 0) return;
            else continue;
        }
    }
}
