#!/bin/bash

# Script de ejecución para la práctica 1: Algoritmos Voraces
# Compila y ejecuta el programa con los casos de prueba

echo "=========================================="
echo "Práctica 1: Algoritmos Voraces - String Art"
echo "=========================================="
echo ""

# Compilar el programa
echo "Compilando programa..."
gcc -o hilos code/hilos.c -lm -std=c99
if [ $? -ne 0 ]; then
    echo "Error: Fallo en la compilación"
    exit 1
fi
echo "Compilación exitosa"
echo ""

# Crear directorio de resultados si no existe
mkdir -p resultados

# Caso de prueba 1: Imagen pequeña con pocos clavos
echo "----------------------------------------"
echo "Caso de prueba 1: messi.pgm (n=100, p=200, s=10)"
echo "----------------------------------------"
./hilos 100 200 10 imagenes/pixel/messi.pgm resultados/messi_resultado.pgm
echo ""

# Caso de prueba 2: Imagen pequeña con más clavos
echo "----------------------------------------"
echo "Caso de prueba 2: ilita-topuria.pgm (n=200, p=400, s=20)"
echo "----------------------------------------"
./hilos 200 400 20 imagenes/pixel/ilita-topuria.pgm resultados/ilita_resultado.pgm
echo ""

# Caso de prueba 3: Parámetros del enunciado
echo "----------------------------------------"
echo "Caso de prueba 3: messi.pgm (n=1000, p=1100, s=30) - Parámetros del enunciado"
echo "----------------------------------------"
./hilos 1000 1100 30 imagenes/pixel/messi.pgm resultados/messi_enunciado.pgm
echo ""

echo "=========================================="
echo "Ejecución completada"
echo "=========================================="
echo "Resultados guardados en: resultados/"
echo ""
