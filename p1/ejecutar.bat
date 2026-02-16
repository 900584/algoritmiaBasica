@echo off
REM Script de ejecución para Windows
REM Compila y ejecuta el programa con los casos de prueba

echo ==========================================
echo Practica 1: Algoritmos Voraces - String Art
echo ==========================================
echo.

REM Compilar el programa
echo Compilando programa...
gcc -o hilos.exe code\hilos.c -lm -std=c99
if %errorlevel% neq 0 (
    echo Error: Fallo en la compilacion
    echo.
    echo Asegurate de tener GCC instalado (MinGW o MSYS2)
    pause
    exit /b 1
)
echo Compilacion exitosa
echo.

REM Crear directorio de resultados si no existe
if not exist resultados mkdir resultados

REM Caso de prueba 1: Imagen pequeña con pocos clavos
echo ----------------------------------------
echo Caso de prueba 1: messi.pgm (n=100, p=200, s=10)
echo ----------------------------------------
hilos.exe 100 200 10 imagenes\pixel\messi.pgm resultados\messi_resultado.pgm
echo.

REM Caso de prueba 2: Imagen pequeña con más clavos
echo ----------------------------------------
echo Caso de prueba 2: ilita-topuria.pgm (n=200, p=400, s=20)
echo ----------------------------------------
hilos.exe 200 400 20 imagenes\pixel\ilita-topuria.pgm resultados\ilita_resultado.pgm
echo.

REM Caso de prueba 3: Parámetros del enunciado (más rápido para prueba)
echo ----------------------------------------
echo Caso de prueba 3: messi.pgm (n=500, p=600, s=30) - Parametros reducidos para prueba rapida
echo ----------------------------------------
hilos.exe 500 600 30 imagenes\pixel\messi.pgm resultados\messi_enunciado.pgm
echo.

echo ==========================================
echo Ejecucion completada
echo ==========================================
echo Resultados guardados en: resultados\
echo.
pause
