#!/bin/bash

# ============================================================================
# Script de prueba para verificar las tasks de Zed
# Team Virus Payal - ASOC 2025
# ============================================================================

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Banner
echo -e "${CYAN}"
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║     🐚 SCRIPT DE PRUEBA BASH - SIMPLEZ-13 PROJECT           ║"
echo "║     Team Virus Payal                                         ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Función para verificar comandos
check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "${GREEN}✓${NC} $1 encontrado: $(which $1)"
    else
        echo -e "${RED}✗${NC} $1 no encontrado"
    fi
}

# Información del sistema
echo -e "${BLUE}💻 Información del sistema:${NC}"
echo "   Sistema: $(uname -s)"
echo "   Arquitectura: $(uname -m)"
echo "   Usuario: $(whoami)"
echo "   Fecha: $(date '+%Y-%m-%d %H:%M:%S')"
echo ""

# Verificar comandos necesarios
echo -e "${BLUE}🔍 Verificando herramientas instaladas:${NC}"
check_command "bash"
check_command "python3"
check_command "make"
check_command "gcc"
check_command "clang"
check_command "git"
echo ""

# Verificar estructura del proyecto
echo -e "${BLUE}📁 Verificando estructura del proyecto:${NC}"

FILES=(
    "../simplez.h"
    "../instrucciones.c"
    "../main.c"
    "../Makefile"
    "../run_example.sh"
    "README.md"
    "INSTRUCCIONES.md"
    "RESUMEN.md"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        SIZE=$(ls -lh "$file" | awk '{print $5}')
        echo -e "   ${GREEN}✓${NC} $file (${SIZE})"
    else
        echo -e "   ${RED}✗${NC} $file"
    fi
done
echo ""

# Contar programas de ejemplo
echo -e "${BLUE}📚 Programas de ejemplo disponibles:${NC}"
EXAMPLE_COUNT=$(ls -1 [0-9][0-9]_*.c 2>/dev/null | wc -l)
echo "   Total: ${EXAMPLE_COUNT} programas"

if [ $EXAMPLE_COUNT -gt 0 ]; then
    echo ""
    ls -1 [0-9][0-9]_*.c | while read file; do
        echo "   • $file"
    done
fi
echo ""

# Test simple de operaciones
echo -e "${BLUE}🧪 Ejecutando prueba simple:${NC}"
A=5
B=3
RESULT=$((A + B))
echo "   Operación: $A + $B = $RESULT"

if [ $RESULT -eq 8 ]; then
    echo -e "   ${GREEN}✓ Test exitoso!${NC}"
else
    echo -e "   ${RED}✗ Test fallido${NC}"
fi
echo ""

# Verificar permisos de run_example.sh
echo -e "${BLUE}🔐 Verificando permisos:${NC}"
if [ -x "../run_example.sh" ]; then
    echo -e "   ${GREEN}✓${NC} run_example.sh es ejecutable"
else
    echo -e "   ${YELLOW}⚠${NC}  run_example.sh no es ejecutable"
    echo "   Tip: chmod +x ../run_example.sh"
fi
echo ""

# Información de Git (si está en un repo)
if [ -d "../../.git" ]; then
    echo -e "${BLUE}📦 Información del repositorio Git:${NC}"
    cd ../..
    echo "   Branch: $(git branch --show-current 2>/dev/null || echo 'N/A')"
    echo "   Último commit: $(git log -1 --oneline 2>/dev/null || echo 'N/A')"
    cd - > /dev/null
    echo ""
fi

# Espacio en disco
echo -e "${BLUE}💾 Espacio en disco:${NC}"
df -h . | tail -1 | awk '{print "   Disponible: "$4" de "$2" ("$5" usado)"}'
echo ""

# Resumen final
echo -e "${CYAN}╔═══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║              ${GREEN}✅ SCRIPT EJECUTADO CORRECTAMENTE${CYAN}               ║${NC}"
echo -e "${CYAN}╚═══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}Para ejecutar este script desde Zed:${NC}"
echo "  1. Presiona Cmd+Shift+P (Mac) o Ctrl+Shift+P (Linux/Windows)"
echo "  2. Escribe 'Tasks: Spawn'"
echo "  3. Selecciona '🐚 Ejecutar Shell Script'"
echo ""
echo -e "${YELLOW}O desde la terminal:${NC}"
echo "  bash test_script.sh"
echo "  # o"
echo "  chmod +x test_script.sh && ./test_script.sh"
echo ""

# Exit con código de éxito
exit 0
