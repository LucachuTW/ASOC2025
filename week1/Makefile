# --- Variables de Compilación ---

# Compilador de C a utilizar
CC = gcc

# Banderas (flags) para el compilador:
# -g:         Incluye información de depuración (para gdb)
# -Wall:      Activa todas las advertencias (Warning All)
# -Wextra:    Activa advertencias adicionales
# -std=c99:   Usa el estándar de C99 (compatible con el código)
CFLAGS = -g -Wall -Wextra -std=c99

# Banderas para el enlazador (Linker)
LDFLAGS =

# --- Variables del Proyecto ---

# Nombre del ejecutable final
TARGET = emulador

# Lista de archivos objeto (.o) que se generarán
# Makefile automáticamente deduce que main.o viene de main.c
OBJS = main.o instrucciones.o

# --- Reglas de Compilación ---

# La regla 'all' es la regla por defecto que se ejecuta si solo escribes 'make'
# Depende del ejecutable final (TARGET)
all: $(TARGET)

# Regla para crear el ejecutable final
# Depende de todos los archivos objeto (OBJS)
# Enlaza (linka) los archivos .o para crear el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)
	@echo "¡Emulador compilado exitosamente como '$(TARGET)'!"

# Regla para compilar main.c en main.o
# Depende de main.c y simplez.h. Si alguno cambia, se recompila.
main.o: main.c simplez.h
	$(CC) $(CFLAGS) -c main.c

# Regla para compilar instrucciones.c en instrucciones.o
# Depende de instrucciones.c y simplez.h.
instrucciones.o: instrucciones.c simplez.h
	$(CC) $(CFLAGS) -c instrucciones.c

# Regla 'clean' para limpiar el directorio
# Elimina el ejecutable y todos los archivos objeto
clean:
	rm -f $(TARGET) $(OBJS)
	@echo "Archivos compilados eliminados."

# Indica que 'all' y 'clean' no son nombres de archivos
.PHONY: all clean