#include <stdint.h>
#include "modlib.h"

#define MOD_BASE ((volatile uint8_t*)0x00120000)
#define VGA_BASE 0x000B8000

void module_entry(void){
    uint8_t txt = 0x0A; // verde
    uint8_t tit = 0x0B; // cyan claro
    uint8_t attr_bg = 0x07; // gris claro sobre negro

    cls(attr_bg);
    print_xy("[MODULO C] Demo diagnostico", 2, 1, tit);
    print_xy("(Pulsa cualquier tecla para volver)", 2, 2, tit);

    // Mostrar direcciones relevantes
    print_label_hex("MOD_BASE:", (uint32_t)MOD_BASE, 4, tit, txt);
    print_label_hex("VGA:", (uint32_t)VGA_BASE, 5, txt, tit);

    // Prueba: contar bytes cero en primeros 256 bytes a partir de inicio REAL del módulo
    const uint8_t* pk = (const uint8_t*)MOD_BASE;
    int zeros = 0;
    for(int i=0;i<256;++i){ if(pk[i]==0) zeros++; }

    // Mostrar resultado (zeros en decimal)
    char zbuf[12];
    u32_to_dec((uint32_t)zeros, zbuf);
    print_xy("Zeros en primeros 256 bytes:", 2, 10, txt);
    print_xy(zbuf, 32, 10, tit);

    // Pequeña barra de "tarea" completada (visual, no permite salir)
    for(int i=0;i<30;i++){
        putc_xy('=', 2+i, 12, tit);
        delay(250000);
    }

    // Indicador final: esperar explícitamente una tecla para salir.
    print_xy("Pulsa cualquier tecla para volver...", 2, 14, tit);
    mod_wait_for_keypress();

    // Mensaje de despedida breve antes del return
    print_xy("Regresando al PseudoKernel...", 2, 15, txt);
    delay(200000); // pequeño retardo visual
    return; // asegurar ret
}
