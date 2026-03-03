#!/bin/bash

# ejecutar.sh - Practica 2: Programacion Dinamica - Particion de palabras
# Compila el programa y ejecuta los casos de prueba
# Ejecutar desde la carpeta practica2_900085_900584: bash ejecutar.sh

echo "=========================================="
echo "Practica 2: Programacion Dinamica - Particion de palabras"
echo "=========================================="
echo ""

# Compilar
echo "Compilando programa separarPalabras..."
gcc -O2 -o separarPalabras code/separarPalabras.c -std=c99
if [ $? -ne 0 ]; then
    echo "Error: Fallo en la compilacion"
    exit 1
fi
echo "Compilacion exitosa"
echo ""

# Caso 1: megusta - Una sola particion
echo "------------------------------------------"
echo "Caso 1: megusta (variante 1 - recursiva)"
echo "------------------------------------------"
./separarPalabras 1 pruebas/diccionario_ejemplo.txt pruebas/texto_megusta.txt
echo ""

echo "------------------------------------------"
echo "Caso 1b: megusta (variante 2 - con memoria)"
echo "------------------------------------------"
./separarPalabras 2 pruebas/diccionario_ejemplo.txt pruebas/texto_megusta.txt
echo ""

echo "------------------------------------------"
echo "Caso 1c: megusta (variante 3 - tabla)"
echo "------------------------------------------"
./separarPalabras 3 pruebas/diccionario_ejemplo.txt pruebas/texto_megusta.txt
echo ""

# Caso 2: megustasoldar - Varias particiones
echo "------------------------------------------"
echo "Caso 2: megustasoldar - multiples particiones (variante 1)"
echo "------------------------------------------"
./separarPalabras 1 pruebas/diccionario_ejemplo.txt pruebas/texto_megustasoldar.txt
echo ""

# Caso 3: megustadolar - No se puede particionar
echo "------------------------------------------"
echo "Caso 3: megustadolar - No se puede particionar"
echo "------------------------------------------"
./separarPalabras 1 pruebas/diccionario_ejemplo.txt pruebas/texto_megustadolar.txt
echo ""

# Caso 4: palabra simple - sol
echo "------------------------------------------"
echo "Caso 4: sol (palabra unica)"
echo "------------------------------------------"
./separarPalabras 3 pruebas/diccionario_ejemplo.txt pruebas/texto_solo.txt
echo ""

# Caso 5: helado
echo "------------------------------------------"
echo "Caso 5: helado (palabra unica)"
echo "------------------------------------------"
./separarPalabras 3 pruebas/diccionario_ejemplo.txt pruebas/texto_helado.txt
echo ""

echo "=========================================="
echo "Ejecucion completada"
echo "=========================================="
