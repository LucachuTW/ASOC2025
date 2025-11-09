/* Módulo post en C: encabezado simple (magic, versión, longitud, payload)
   Compilado sin runtime, convertido a binario y empaquetado como 1 sector. */

const char magic[4] = {'M','O','D','0'};
const unsigned short version = 1;
const unsigned short length = 16;
const char payload[16] = "Hola desde MOD0";

/* Mantenerlo simple: todo queda en .data por default.
   El Makefile hará objcopy -O binary y pad a 512 bytes. */
