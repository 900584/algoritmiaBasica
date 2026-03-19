#!/bin/bash

# ejecutar.sh - Practica 3: Busqueda con retroceso - Ubicacion de centros de urgencias
# Compila el programa y ejecuta los casos de prueba
# Ejecutar desde la carpeta practica3_900085_900584: bash ejecutar.sh

echo "=========================================="
echo "Practica 3: Busqueda con retroceso"
echo "Ubicacion de centros de urgencias"
echo "=========================================="
echo ""

# Compilar
echo "Compilando programa ubicaCentros..."
gcc -O2 -o ubicaCentros code/ubicaCentros.c -std=c99 -lm
if [ $? -ne 0 ]; then
    echo "Error: Fallo en la compilacion"
    exit 1
fi
echo "Compilacion exitosa"
echo ""

mkdir -p resultados

# ======================================================================
# SECCION 1: VERIFICACION CON EJEMPLOS DEL ENUNCIADO
# ======================================================================

echo "=========================================="
echo "VERIFICACION: Ejemplos del enunciado"
echo "=========================================="
echo ""

echo "Ejecutando ejemplos del enunciado..."
./ubicaCentros pruebas/ejemplo_enunciado.txt resultados/resultado_enunciado.txt
echo "Resultado:"
cat resultados/resultado_enunciado.txt
echo ""
echo "Esperado caso 1 (5 nodos, 1 centro existente en 1, k=1):"
echo "  Valor optimo: 10, nuevo centro en localidad 5"
echo "Esperado caso 2 (6 nodos, centros en 2 y 5, k=2):"
echo "  Valor optimo: 3, nuevos centros en localidades 1 y 3"
echo ""

# ======================================================================
# SECCION 2: CASOS TRIVIALES
# ======================================================================

echo "=========================================="
echo "VERIFICACION: Casos triviales"
echo "=========================================="
echo ""

echo "Ejecutando casos triviales..."
./ubicaCentros pruebas/caso_trivial.txt resultados/resultado_trivial.txt
echo "Resultado:"
cat resultados/resultado_trivial.txt
echo ""

# ======================================================================
# SECCION 3: CASO MEDIANO
# ======================================================================

echo "=========================================="
echo "VERIFICACION: Caso mediano (10 nodos)"
echo "=========================================="
echo ""

echo "Ejecutando caso mediano..."
./ubicaCentros pruebas/caso_mediano.txt resultados/resultado_mediano.txt
echo "Resultado:"
cat resultados/resultado_mediano.txt
echo ""

# ======================================================================
# SECCION 4: EXPERIMENTOS
# ======================================================================

echo "=========================================="
echo "EXPERIMENTOS"
echo "=========================================="
echo ""

# Generar ficheros de prueba para experimentacion
echo "Generando ficheros de prueba (requiere Python3)..."
python3 tools/generar_pruebas.py 2>/dev/null || python tools/generar_pruebas.py
if [ $? -ne 0 ]; then
    echo "Error: Fallo al generar ficheros de prueba."
    echo "Comprueba que Python3 esta disponible."
    exit 1
fi
echo ""

echo "------------------------------------------"
echo "Experimento 1: Variar n (num. localidades) con k=2"
echo "------------------------------------------"
./ubicaCentros pruebas/exp_variar_n.txt resultados/resultado_variar_n.txt
echo "Resultados (tiempo_ms nodos valor_optimo solucion):"
cat resultados/resultado_variar_n.txt
echo ""

echo "------------------------------------------"
echo "Experimento 2: Variar k (nuevos centros) con n=15"
echo "------------------------------------------"
./ubicaCentros pruebas/exp_variar_k.txt resultados/resultado_variar_k.txt
echo "Resultados:"
cat resultados/resultado_variar_k.txt
echo ""

echo "------------------------------------------"
echo "Experimento 3: Casos grandes"
echo "------------------------------------------"
./ubicaCentros pruebas/exp_grande.txt resultados/resultado_grande.txt
echo "Resultados:"
cat resultados/resultado_grande.txt
echo ""

echo "=========================================="
echo "Ejecucion completada"
echo "=========================================="
echo "Resultados guardados en: resultados/"
echo ""
