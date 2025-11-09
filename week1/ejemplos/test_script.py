#!/usr/bin/env python3
"""
Script de prueba para verificar las tasks de Zed
Team Virus Payal - ASOC 2025
"""

import sys
import os
from datetime import datetime


def print_banner():
    """Imprime un banner ASCII"""
    banner = """
╔═══════════════════════════════════════════════════════════════╗
║     🐍 SCRIPT DE PRUEBA PYTHON3 - SIMPLEZ-13 PROJECT        ║
║     Team Virus Payal                                         ║
╚═══════════════════════════════════════════════════════════════╝
    """
    print(banner)


def check_python_version():
    """Verifica la versión de Python"""
    version = sys.version_info
    print(f"✓ Python Version: {version.major}.{version.minor}.{version.micro}")
    print(f"✓ Ejecutable: {sys.executable}")
    print()


def check_project_structure():
    """Verifica la estructura del proyecto"""
    print("📁 Verificando estructura del proyecto...")

    # Obtener directorio actual
    current_dir = os.getcwd()
    print(f"   Directorio actual: {current_dir}")

    # Archivos importantes
    important_files = [
        "../simplez.h",
        "../instrucciones.c",
        "../main.c",
        "../Makefile",
        "../run_example.sh",
        "README.md",
        "INSTRUCCIONES.md",
    ]

    print("\n   Verificando archivos:")
    for file in important_files:
        exists = "✓" if os.path.exists(file) else "✗"
        print(f"   {exists} {file}")
    print()


def show_examples():
    """Muestra los ejemplos disponibles"""
    print("📚 Programas de ejemplo disponibles:")

    examples = []
    for i in range(1, 11):
        filename = f"{i:02d}_programa_*.c"
        # Buscar archivos que coincidan
        import glob

        matches = glob.glob(filename)
        if matches:
            examples.append((i, os.path.basename(matches[0])))

    for num, name in examples:
        print(f"   {num:2d}. {name}")
    print()


def run_simple_test():
    """Ejecuta una prueba simple"""
    print("🧪 Ejecutando prueba simple...")

    # Test de operaciones básicas
    a = 5
    b = 3
    result = a + b

    print(f"   Operación: {a} + {b} = {result}")
    print(f"   ✓ Test exitoso!")
    print()


def show_system_info():
    """Muestra información del sistema"""
    print("💻 Información del sistema:")
    print(f"   Sistema Operativo: {sys.platform}")
    print(f"   Fecha/Hora: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print()


def main():
    """Función principal"""
    print_banner()
    check_python_version()
    show_system_info()
    check_project_structure()
    show_examples()
    run_simple_test()

    print("╔═══════════════════════════════════════════════════════════════╗")
    print("║              ✅ SCRIPT EJECUTADO CORRECTAMENTE               ║")
    print("╚═══════════════════════════════════════════════════════════════╝")
    print()
    print("Para ejecutar este script desde Zed:")
    print("  1. Presiona Cmd+Shift+P (Mac) o Ctrl+Shift+P (Linux/Windows)")
    print("  2. Escribe 'Tasks: Spawn'")
    print("  3. Selecciona '🐍 Ejecutar Python (python3)'")
    print()
    print("O desde la terminal:")
    print("  python3 test_script.py")
    print()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n⚠️  Ejecución interrumpida por el usuario")
        sys.exit(0)
    except Exception as e:
        print(f"\n❌ Error: {e}")
        sys.exit(1)
