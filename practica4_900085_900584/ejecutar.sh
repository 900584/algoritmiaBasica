#!/usr/bin/env bash
set -euo pipefail

echo "[1/4] Compilando..."
make clean >/dev/null 2>&1 || true
make

echo "[2/4] Ejecutando Branch&Bound..."
./formarEquipos pruebas/pruebas.txt resultados/resultados_pruebas.txt
./formarEquipos pruebas/experimentos.txt resultados/resultados_experimentos_bnb.txt

echo "[3/4] Verificando con fuerza bruta (instancias pequenas)..."
./verificarBruteforce pruebas/pruebas.txt resultados/resultados_pruebas_bruteforce.txt

echo "[4/4] Comparacion basica de valores optimos..."
bnb_vals=$(grep -E "^Valor_optimo=" resultados/resultados_pruebas.txt | sed 's/Valor_optimo=//')
bf_vals=$(grep -E "^Valor_optimo=" resultados/resultados_pruebas_bruteforce.txt | sed 's/Valor_optimo=//')

if [[ "$bnb_vals" == "$bf_vals" ]]; then
  echo "OK: Branch&Bound coincide con fuerza bruta en pruebas/pruebas.txt"
else
  echo "ATENCION: discrepancia detectada entre Branch&Bound y fuerza bruta"
  echo "  BnB: $bnb_vals"
  echo "  BF : $bf_vals"
fi

echo
echo "Ficheros generados:"
echo " - resultados/resultados_pruebas.txt"
echo " - resultados/resultados_experimentos_bnb.txt"
echo " - resultados/resultados_pruebas_bruteforce.txt"
