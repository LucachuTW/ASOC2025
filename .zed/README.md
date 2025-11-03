# 🎯 Tasks de Zed - Documentación

Este archivo explica cómo usar las tasks configuradas para el proyecto Simplez-13 en Zed Editor.

## 📋 Tasks Disponibles

### 🐍 Python

#### 1. "🐍 Ejecutar Python (python3)"
Ejecuta el archivo Python actual usando `python3`.

**Uso:**
- Abre cualquier archivo `.py`
- Presiona `Cmd+Shift+P` (Mac) o `Ctrl+Shift+P` (Linux/Windows)
- Escribe "task" y selecciona "Tasks: Spawn"
- Elige "🐍 Ejecutar Python (python3)"

**Ejemplo:**
```python
# mi_script.py
print("¡Hola desde Python!")
```

---

### 🐚 Shell Scripts

#### 2. "🐚 Ejecutar Shell Script"
Ejecuta el script bash/shell actual.

**Uso:**
- Abre cualquier archivo `.sh`
- Presiona `Cmd+Shift+P`
- Selecciona "🐚 Ejecutar Shell Script"

**Ejemplo:**
```bash
#!/bin/bash
echo "¡Hola desde Bash!"
```

#### 3. "✅ Hacer ejecutable y correr (.sh)"
Le da permisos de ejecución al script y lo ejecuta.

**Uso:** Útil cuando el script no tiene permisos de ejecución.

---

### 🔨 Compilación y Ejecución Simplez-13

#### 4. "🔨 Compilar y Ejecutar Simplez-13"
Compila el proyecto Simplez-13 y ejecuta el emulador.

**Pasos que realiza:**
1. `cd Exercices`
2. `make clean`
3. `make`
4. `./emulador`

**Uso ideal:** Después de modificar `main.c` o archivos del emulador.

#### 5. "🎯 Ejecutar Ejemplo Simplez-13"
Ejecuta el script de selección de ejemplos.

**Lo que hace:**
```bash
cd Exercices
./run_example.sh
```

Te mostrará el menú interactivo para elegir qué ejemplo ejecutar.

#### 6. "📝 Compilar solo (make)"
Solo compila sin ejecutar.

**Uso:** Verificar que no hay errores de compilación.

#### 7. "🧹 Limpiar compilación (make clean)"
Limpia archivos objeto y ejecutables.

**Uso:** Antes de una compilación limpia o para resolver problemas.

---

## ⌨️ Atajos Rápidos

### En Zed:

| Acción | Atajo |
|--------|-------|
| Abrir Tasks | `Cmd+Shift+P` → "Tasks: Spawn" |
| Reejecutar última task | `Cmd+Shift+P` → "Tasks: Rerun" |
| Terminal | `Ctrl+`` (backtick) |

---

## 🎯 Flujo de Trabajo Recomendado

### Para desarrollar un nuevo programa:

1. Crea tu archivo en `Exercices/ejemplos/mi_programa.c`
2. Copia a main.c: `cp ejemplos/mi_programa.c main.c`
3. Ejecuta task: "🔨 Compilar y Ejecutar Simplez-13"
4. Prueba y depura

### Para probar ejemplos existentes:

1. Ejecuta task: "🎯 Ejecutar Ejemplo Simplez-13"
2. Selecciona el número del programa
3. Presiona Enter para avanzar paso a paso

### Para scripts Python:

1. Crea tu archivo `.py`
2. Abre el archivo en Zed
3. Ejecuta task: "🐍 Ejecutar Python (python3)"

### Para scripts Bash:

1. Crea tu archivo `.sh`
2. Abre el archivo en Zed
3. Ejecuta task: "✅ Hacer ejecutable y correr (.sh)"

---

## 🔧 Variables de Entorno en Tasks

Las tasks usan estas variables de Zed:

- `$ZED_FILE` - Ruta completa del archivo actual
- `$ZED_WORKTREE_ROOT` - Raíz del proyecto (donde está ASOC2025)
- `$ZED_COLUMN` - Columna del cursor
- `$ZED_ROW` - Fila del cursor

---

## 🐛 Solución de Problemas

### "comando no encontrado: python3"
Verifica que Python 3 esté instalado:
```bash
which python3
```

Si no está instalado:
```bash
# macOS
brew install python3

# Linux (Ubuntu/Debian)
sudo apt install python3
```

### "Permission denied" al ejecutar scripts
Usa la task "✅ Hacer ejecutable y correr (.sh)" que automáticamente:
```bash
chmod +x script.sh
```

### El emulador no compila
1. Ejecuta task "🧹 Limpiar compilación"
2. Verifica que `simplez.h` e `instrucciones.c` existan
3. Ejecuta task "📝 Compilar solo" para ver errores

---

## 📚 Recursos Adicionales

- **Documentación de Zed Tasks:** https://zed.dev/docs/tasks
- **Ejemplos del proyecto:** `Exercices/ejemplos/`
- **Instrucciones completas:** `Exercices/ejemplos/INSTRUCCIONES.md`

---

## 🎨 Personalización

Para añadir tus propias tasks, edita `.zed/tasks.json`:

```json
{
    "label": "Mi Task Personalizada",
    "command": "comando",
    "args": ["arg1", "arg2"],
    "use_new_terminal": true,
    "reveal": "always"
}
```

---

**Team Virus Payal** 🖤  
*Efficient workflows for efficient code.*