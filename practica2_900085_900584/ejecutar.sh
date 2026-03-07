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

# ==========================================================================
# SECCION DE EXPERIMENTOS (Tarea 3)
# Compara las tres variantes con diccionarios y textos de distintos tamanios
# ==========================================================================

echo ""
echo "=========================================="
echo "EXPERIMENTOS: Comparativa de variantes"
echo "=========================================="
echo ""

# Generar ficheros de prueba grandes con el script Python
echo "Generando ficheros de prueba (requiere Python3)..."
python3 tools/generar_pruebas.py
if [ $? -ne 0 ]; then
    echo "Error: Fallo al generar ficheros de prueba. Comprueba que Python3 esta disponible."
    exit 1
fi
echo ""

# Para cada tamanio de diccionario, ejecutar las 3 variantes y medir tiempo
for TAM in 100 1000 5000; do
    DICC="pruebas/diccionario_${TAM}.txt"
    TEXTO_VALIDO="pruebas/texto_${TAM}_valido.txt"
    TEXTO_MUTADO="pruebas/texto_${TAM}_mutado.txt"

    echo "=========================================="
    echo "Diccionario: ${TAM} palabras  |  Texto VALIDO (segmentable)"
    echo "=========================================="

    for VAR in 1 2 3; do
        echo "  -- Variante ${VAR} --"
        # Todas las variantes tienen timeout: la 1 por coste exponencial en la decision,
        # la 2 y 3 porque la enumeracion de TODAS las particiones puede ser exponencial
        # si hay muchas combinaciones validas (independientemente de la variante).
        { time timeout 60 ./separarPalabras ${VAR} "${DICC}" "${TEXTO_VALIDO}"; } 2>&1
        RET=$?
        if [ ${RET} -eq 124 ]; then
            echo "  [timeout: variante ${VAR} supero 60s con ${TAM} palabras]"
        fi
        echo ""
    done

    echo "=========================================="
    echo "Diccionario: ${TAM} palabras  |  Texto MUTADO (puede no ser segmentable)"
    echo "=========================================="

    for VAR in 1 2 3; do
        echo "  -- Variante ${VAR} --"
        { time timeout 60 ./separarPalabras ${VAR} "${DICC}" "${TEXTO_MUTADO}"; } 2>&1
        RET=$?
        if [ ${RET} -eq 124 ]; then
            echo "  [timeout: variante ${VAR} supero 60s con ${TAM} palabras]"
        fi
        echo ""
    done

done

echo "=========================================="
echo "Experimentos completados"
echo "=========================================="
