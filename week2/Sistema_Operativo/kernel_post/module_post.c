/* Módulo post en C: encabezado simple (magic, versión, longitud, payload)
   Compilado sin runtime, convertido a binario y empaquetado como 1 sector. */

// Usar una estructura para garantizar el orden
struct module_header {
    char magic[4];
    unsigned short version;
    unsigned short length;
    char payload[16];
} __attribute__((packed));

const struct module_header module = {
    .magic = {'M','O','D','0'},
    .version = 1,
    .length = 16,
    .payload = "Hola desde MOD0"
};

/* Mantenerlo simple: todo queda en .data por default.
   El Makefile hará objcopy -O binary y pad a 512 bytes. */
