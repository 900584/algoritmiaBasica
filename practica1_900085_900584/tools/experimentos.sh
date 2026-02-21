#!/bin/bash

# experimentos.sh - Tarea 3: Experimentacion
# Practica 1: Algoritmos Voraces - String Art
#
# 5 baterias de experimentos:
#   EXP1 - Variacion de n (numero de clavos)
#   EXP2 - Variacion de p (candidatos por iteracion)
#   EXP3 - Variacion de s (mejores hilos seleccionados)
#   EXP4 - Diversas imagenes con parametros fijos
#   EXP5 - Parametros del enunciado (n=1000, p=1100, s=30)
#
# Ejecutar desde la carpeta p1/:  bash tools/experimentos.sh
#
# Resultados: experimentos/resultados.csv
#             experimentos/imagenes/

BINARY=./hilos
IMG_DIR=pruebas
OUT_DIR=experimentos
IMG_OUT=$OUT_DIR/imagenes
CSV=$OUT_DIR/resultados.csv

# ─── Compilar ──────────────────────────────────────────────────────────────
echo "=========================================="
echo " Compilando..."
echo "=========================================="
gcc -O2 -o hilos code/hilos.c -lm -std=c99
if [ $? -ne 0 ]; then
    echo "ERROR: Fallo en la compilacion"
    exit 1
fi
echo "Compilacion exitosa"
echo ""

mkdir -p "$IMG_OUT"

# ─── Cabecera CSV ──────────────────────────────────────────────────────────
echo "experimento,imagen,n,p,s,hilos_dibujados,error_final,tiempo_seg" > "$CSV"

# ─── Funcion auxiliar ──────────────────────────────────────────────────────
run_exp() {
    local exp_label="$1"
    local img_name="$2"
    local n="$3"
    local p="$4"
    local s="$5"

    local input="$IMG_DIR/${img_name}.pgm"
    local output="$IMG_OUT/${exp_label}_${img_name}_n${n}_p${p}_s${s}.pgm"

    echo "  >> $exp_label | img=$img_name  n=$n  p=$p  s=$s"

    # Ejecutar y capturar salida
    local salida
    salida=$($BINARY "$n" "$p" "$s" "$input" "$output" 2>/dev/null)

    # Extraer metricas (grep -oE compatible con MSYS2/Git Bash)
    local hilos
    hilos=$(echo "$salida" | grep "Numero de hilos dibujados" | grep -oE '[0-9]+' | tail -1)
    local error
    error=$(echo "$salida" | grep "Error final" | grep -oE '[0-9]+[.]?[0-9]*' | tail -1)
    local tiempo
    tiempo=$(echo "$salida" | grep "Tiempo de ejecucion" | grep -oE '[0-9]+[.]?[0-9]*' | tail -1)

    # Valores por defecto si algo falla
    hilos=${hilos:-0}
    error=${error:-0}
    tiempo=${tiempo:-0}

    echo "     hilos=$hilos  error=$error  tiempo=${tiempo}s"

    # Escribir en CSV
    echo "$exp_label,$img_name,$n,$p,$s,$hilos,$error,$tiempo" >> "$CSV"
}

# =============================================================================
# EXP1: Variacion de n  (p=500, s=20, imagen=messi)
# Como afecta el numero de clavos a la calidad y al tiempo?
# =============================================================================
echo "=========================================="
echo " EXP1: Variacion de n  (p fijo=500, s fijo=20)"
echo "=========================================="
for n in 50 100 200 400 600 1000; do
    run_exp "exp1_var_n" "messi" "$n" 500 20
done
echo ""

# =============================================================================
# EXP2: Variacion de p  (n=200, s=20, imagen=messi)
# Como afecta el pool de candidatos a la calidad y al tiempo?
# =============================================================================
echo "=========================================="
echo " EXP2: Variacion de p  (n fijo=200, s fijo=20)"
echo "=========================================="
for p in 50 100 300 600 1000 2000; do
    run_exp "exp2_var_p" "messi" 200 "$p" 20
done
echo ""

# =============================================================================
# EXP3: Variacion de s  (n=200, p=500, imagen=messi)
# Como afecta elegir mas o menos hilos por iteracion?
# =============================================================================
echo "=========================================="
echo " EXP3: Variacion de s  (n fijo=200, p fijo=500)"
echo "=========================================="
for s in 5 10 20 50 100 200; do
    run_exp "exp3_var_s" "messi" 200 500 "$s"
done
echo ""

# =============================================================================
# EXP4: Diversas imagenes  (n=200, p=500, s=20)
# Como afecta el contenido de la imagen al rendimiento?
# =============================================================================
echo "=========================================="
echo " EXP4: Diversas imagenes  (n=200, p=500, s=20)"
echo "=========================================="
for img in messi iliaTopuria emoji qr; do
    run_exp "exp4_imagenes" "$img" 200 500 20
done
echo ""

# =============================================================================
# EXP5: Parametros del enunciado exactos  (n=1000, p=1100, s=30)
# Benchmark de referencia con los valores del enunciado
# =============================================================================
echo "=========================================="
echo " EXP5: Parametros del enunciado (n=1000, p=1100, s=30)"
echo "=========================================="
run_exp "exp5_enunciado" "messi"        1000 1100 30
run_exp "exp5_enunciado" "iliaTopuria"  1000 1100 30
run_exp "exp5_enunciado" "emoji"        1000 1100 30
run_exp "exp5_enunciado" "qr"           1000 1100 30
echo ""

# ─── Resumen ───────────────────────────────────────────────────────────────
echo "=========================================="
echo " EXPERIMENTOS COMPLETADOS"
echo "=========================================="
echo "  CSV con resultados : $CSV"
echo "  Imagenes generadas : $IMG_OUT/"
echo ""
echo " Vista del CSV:"
echo "------------------------------------------"
cat "$CSV"
echo "------------------------------------------"
