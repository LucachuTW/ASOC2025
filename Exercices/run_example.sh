#!/bin/bash

# ============================================================================
# Script para ejecutar programas de ejemplo del Emulador Simplez-13
# Team Virus Payal - ASOC 2025
# ============================================================================

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Banner
echo -e "${CYAN}"
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║     EMULADOR SIMPLEZ-13 - SELECTOR DE PROGRAMAS              ║"
echo "║     Team Virus Payal                                         ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Verificar si se pasó un argumento
if [ -z "$1" ]; then
    echo -e "${YELLOW}Uso: ./run_example.sh [número]${NC}"
    echo ""
    echo "Programas disponibles:"
    echo ""
    echo -e "${GREEN}Nivel Básico:${NC}"
    echo "  1 - Operaciones Básicas (LD #, ST, ADD)"
    echo "  2 - Contador Ascendente (bucles, BZ, SUB #)"
    echo ""
    echo -e "${GREEN}Nivel Intermedio:${NC}"
    echo "  3 - Suma de Array (indexación)"
    echo "  4 - Direccionamiento Indirecto (punteros)"
    echo "  6 - Búsqueda del Máximo (comparación)"
    echo "  7 - Copia de Bloques de Memoria"
    echo ""
    echo -e "${GREEN}Nivel Avanzado:${NC}"
    echo "  5 - Cálculo de Factorial (bucles anidados)"
    echo "  8 - Secuencia de Fibonacci"
    echo " 10 - Simulación de Subrutinas"
    echo ""
    echo -e "${GREEN}Referencia:${NC}"
    echo "  9 - Demostración de Todos los Modos"
    echo ""
    echo -e "${CYAN}Ejemplos:${NC}"
    echo "  ./run_example.sh 1    # Ejecutar programa básico"
    echo "  ./run_example.sh 5    # Ejecutar cálculo de factorial"
    echo ""
    exit 1
fi

# Validar que el número esté entre 1 y 10
if [ "$1" -lt 1 ] || [ "$1" -gt 10 ]; then
    echo -e "${RED}✗ Error: El número debe estar entre 1 y 10${NC}"
    exit 1
fi

# Formatear el número con cero a la izquierda
PROGRAM_NUM=$(printf "%02d" $1)

# Buscar el archivo
PROGRAM_FILE=$(ls ejemplos/${PROGRAM_NUM}_*.c 2>/dev/null | head -n 1)

if [ -z "$PROGRAM_FILE" ]; then
    echo -e "${RED}✗ Error: No se encontró el programa ${PROGRAM_NUM}${NC}"
    exit 1
fi

# Obtener el nombre del programa
PROGRAM_NAME=$(basename "$PROGRAM_FILE" .c | sed 's/^[0-9]*_//')

echo -e "${BLUE}▶ Seleccionado: ${NC}${PROGRAM_NAME}"
echo -e "${BLUE}▶ Archivo: ${NC}${PROGRAM_FILE}"
echo ""

# Hacer backup del main.c actual si existe
if [ -f "main.c" ]; then
    echo -e "${YELLOW}⚠ Creando backup de main.c actual...${NC}"
    cp main.c main.c.backup
fi

# Copiar el programa seleccionado
echo -e "${BLUE}▶ Copiando programa...${NC}"
cp "$PROGRAM_FILE" main.c

# Compilar
echo -e "${BLUE}▶ Limpiando compilación anterior...${NC}"
make clean > /dev/null 2>&1

echo -e "${BLUE}▶ Compilando...${NC}"
if make > /dev/null 2>&1; then
    echo -e "${GREEN}✓ Compilación exitosa${NC}"
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}          Presiona Enter para avanzar paso a paso${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo ""

    # Ejecutar el emulador
    ./emulador

    EXIT_CODE=$?
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    if [ $EXIT_CODE -eq 0 ]; then
        echo -e "${GREEN}✓ Programa finalizado correctamente${NC}"
    else
        echo -e "${YELLOW}⚠ El programa terminó con código de salida: ${EXIT_CODE}${NC}"
    fi
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"

    # Restaurar backup si existe
    if [ -f "main.c.backup" ]; then
        echo ""
        read -p "¿Restaurar el main.c anterior? (s/N): " -n 1 -r
        echo ""
        if [[ $REPLY =~ ^[SsYy]$ ]]; then
            mv main.c.backup main.c
            echo -e "${GREEN}✓ main.c restaurado${NC}"
        else
            rm main.c.backup
            echo -e "${BLUE}▶ Backup eliminado. El nuevo programa permanece en main.c${NC}"
        fi
    fi
else
    echo -e "${RED}✗ Error en la compilación${NC}"
    echo ""
    echo "Detalles del error:"
    make

    # Restaurar backup en caso de error
    if [ -f "main.c.backup" ]; then
        mv main.c.backup main.c
        echo -e "${YELLOW}⚠ main.c restaurado al estado anterior${NC}"
    fi
    exit 1
fi

echo ""
echo -e "${CYAN}Para ejecutar otro programa, usa: ./run_example.sh [1-10]${NC}"
echo ""
