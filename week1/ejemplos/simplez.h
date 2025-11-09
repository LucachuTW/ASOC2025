#ifndef EJEMPLOS_SIMPLEZ_WRAPPER_H
#define EJEMPLOS_SIMPLEZ_WRAPPER_H
#pragma once

/*
  Wrapper de cabecera para los ejemplos.

  Objetivo:
  - Permitir que los programas dentro de ASOC2025/Exercices/ejemplos/
    puedan incluir "simplez.h" sin cambiar las rutas de include ni los
    comandos de compilación.
  - Este archivo reenvía la inclusión al header real ubicado en el
    directorio padre: ASOC2025/Exercices/simplez.h

  Uso:
  - En los ejemplos, simplemente:
      #include "simplez.h"
    y el compilador resolverá este wrapper, que a su vez incluye el header real.

  Nota:
  - Si compilas desde el directorio de ejemplos, el compilador verá primero
    este simplez.h (wrapper) y, mediante la ruta relativa, incluirá el
    archivo original sin necesidad de flags -I adicionales.
*/

#include "../simplez.h"

#endif /* EJEMPLOS_SIMPLEZ_WRAPPER_H */