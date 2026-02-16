# Instrucciones para compilar y ejecutar en Windows

## Requisitos previos

Necesitas tener instalado un compilador de C compatible con GCC. Tienes varias opciones:

### Opción 1: MinGW-w64 (Recomendado)
1. Descarga MinGW-w64 desde: https://www.mingw-w64.org/downloads/
2. O instala a través de MSYS2: https://www.msys2.org/
3. Asegúrate de agregar `bin` al PATH del sistema

### Opción 2: Visual Studio con C/C++
1. Instala Visual Studio Community (gratis)
2. Incluye las herramientas de desarrollo de C/C++
3. Usa el "Developer Command Prompt" para compilar

### Opción 3: WSL (Windows Subsystem for Linux)
Si tienes WSL instalado, puedes usar el entorno Linux directamente.

## Verificar instalación

Abre PowerShell o CMD y ejecuta:
```bash
gcc --version
```

Si muestra la versión, estás listo. Si no, necesitas instalar MinGW.

## Compilación manual

### Método 1: Usando el script batch (Más fácil)

Simplemente ejecuta:
```bash
ejecutar.bat
```

Este script:
- Compila el programa automáticamente
- Crea el directorio `resultados` si no existe
- Ejecuta casos de prueba
- Muestra los resultados

### Método 2: Compilación manual desde CMD/PowerShell

1. Abre CMD o PowerShell en el directorio `p1`

2. Compila el programa:
```bash
gcc -o hilos.exe code\hilos.c -lm -std=c99
```

3. Ejecuta un caso de prueba:
```bash
hilos.exe 100 200 10 imagenes\pixel\messi.pgm resultados\messi_resultado.pgm
```

**Parámetros:**
- `100`: número de clavos (n)
- `200`: número de hilos candidatos por iteración (p)
- `10`: número de mejores hilos a seleccionar (s)
- `imagenes\pixel\messi.pgm`: archivo de entrada
- `resultados\messi_resultado.pgm`: archivo de salida

## Ejecución rápida para pruebas

Para una prueba rápida con parámetros pequeños:

```bash
hilos.exe 50 100 5 imagenes\pixel\messi.pgm resultados\test.pgm
```

Esto debería ejecutarse rápidamente y te permitirá verificar que todo funciona.

## Solución de problemas

### Error: "gcc no se reconoce como comando"
- Instala MinGW-w64 o agrega la ruta al PATH
- Reinicia la terminal después de instalar

### Error: "No se puede abrir el archivo"
- Verifica que las rutas usen `\` en lugar de `/`
- Asegúrate de estar en el directorio correcto (`p1`)

### Error de compilación: "undefined reference to `sqrt`"
- Asegúrate de incluir `-lm` en la compilación
- El comando completo: `gcc -o hilos.exe code\hilos.c -lm -std=c99`

### El programa tarda mucho
- Reduce los parámetros n, p, s para pruebas
- Para imágenes grandes, usa valores más pequeños inicialmente

## Comparación con lab000

En lab000 (Linux), las diferencias principales son:
- Usa `/` en lugar de `\` para rutas
- El ejecutable se llama `hilos` (sin `.exe`)
- El script es `ejecutar.sh` (bash) en lugar de `ejecutar.bat`

El código fuente es el mismo, solo cambia la forma de ejecutarlo.
