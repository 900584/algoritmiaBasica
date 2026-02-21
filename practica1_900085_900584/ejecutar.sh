#!/bin/bash

# ejecutar.sh - Practica 1: Algoritmos Voraces - String Art
# Compila el programa y ejecuta los casos de prueba
# Ejecutar desde la carpeta p1/:  bash ejecutar.sh

echo "=========================================="
echo "Practica 1: Algoritmos Voraces - String Art"
echo "=========================================="
echo ""

# Compilar
echo "Compilando programa..."
gcc -O2 -o hilos code/hilos.c -lm -std=c99
if [ $? -ne 0 ]; then
    echo "Error: Fallo en la compilacion"
    exit 1
fi
echo "Compilacion exitosa"
echo ""

mkdir -p resultados

# Caso 1: messi, parametros pequeños
echo "------------------------------------------"
echo "Caso 1: messi.pgm  (n=100, p=200, s=10)"
echo "------------------------------------------"
./hilos 100 200 10 pruebas/messi.pgm resultados/messi_test1.pgm
echo ""

# Caso 2: iliaTopuria, parametros medianos
echo "------------------------------------------"
echo "Caso 2: iliaTopuria.pgm  (n=200, p=400, s=20)"
echo "------------------------------------------"
./hilos 200 400 20 pruebas/iliaTopuria.pgm resultados/iliaTopuria_test.pgm
echo ""

# Caso 3: emoji, parametros medianos
echo "------------------------------------------"
echo "Caso 3: emoji.pgm  (n=200, p=400, s=20)"
echo "------------------------------------------"
./hilos 200 400 20 pruebas/emoji.pgm resultados/emoji_test.pgm
echo ""

# Caso 4: qr, parametros medianos
echo "------------------------------------------"
echo "Caso 4: qr.pgm  (n=200, p=400, s=20)"
echo "------------------------------------------"
./hilos 200 400 20 pruebas/qr.pgm resultados/qr_test.pgm
echo ""

# Caso 5: messi, parametros del enunciado
echo "------------------------------------------"
echo "Caso 5: messi.pgm  (n=1000, p=1100, s=30) - Parametros del enunciado"
echo "------------------------------------------"
./hilos 1000 1100 30 pruebas/messi.pgm resultados/messi_test2.pgm
echo ""

echo "=========================================="
echo "Ejecucion completada"
echo "=========================================="
echo "Resultados guardados en: resultados/"
echo ""
