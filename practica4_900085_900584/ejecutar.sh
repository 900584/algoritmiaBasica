#!/usr/bin/env bash
set -euo pipefail

echo "[1/6] Compilando..."
make clean >/dev/null 2>&1 || true
make

echo "[2/6] Ejecutando Branch&Bound..."
./formarEquipos pruebas/pruebas.txt resultados/resultados_pruebas.txt
./formarEquipos pruebas/experimentos.txt resultados/resultados_experimentos_bnb.txt

echo "[3/6] Verificando con fuerza bruta (instancias pequenas)..."
./verificarBruteforce pruebas/pruebas.txt resultados/resultados_pruebas_bruteforce.txt

echo "[4/6] Comparacion BnB vs fuerza bruta..."
bnb_vals=$(grep -E "^Valor_optimo=" resultados/resultados_pruebas.txt | sed 's/Valor_optimo=//')
bf_vals=$(grep -E "^Valor_optimo=" resultados/resultados_pruebas_bruteforce.txt | sed 's/Valor_optimo=//')

if [[ "$bnb_vals" == "$bf_vals" ]]; then
  echo "OK: Branch&Bound coincide con fuerza bruta en pruebas/pruebas.txt"
else
  echo "ATENCION: discrepancia detectada entre Branch&Bound y fuerza bruta"
  echo "  BnB: $bnb_vals"
  echo "  BF : $bf_vals"
fi

echo "[5/6] Ejecutando solucion ILP (programacion lineal)..."
if ! command -v python3 &>/dev/null; then
  echo "AVISO: python3 no encontrado, omitiendo Tarea 4."
else
  python3 -c "import pulp" 2>/dev/null || { echo "AVISO: pulp no instalado. Ejecuta: pip install pulp"; exit 0; }
  python3 src/resolverLP.py pruebas/pruebas.txt resultados/resultados_pruebas_lp.txt
  python3 src/resolverLP.py pruebas/experimentos.txt resultados/resultados_experimentos_lp.txt
  echo "ILP ejecutado correctamente."

  echo "[6/6] Comparacion BnB vs ILP..."
  lp_vals=$(grep -E "^Valor_optimo=" resultados/resultados_pruebas_lp.txt | sed 's/Valor_optimo=//')
  if [[ "$bnb_vals" == "$lp_vals" ]]; then
    echo "OK: Branch&Bound coincide con ILP en pruebas/pruebas.txt"
  else
    echo "ATENCION: discrepancia entre Branch&Bound e ILP"
    echo "  BnB: $bnb_vals"
    echo "  ILP: $lp_vals"
  fi
fi

echo
echo "Ficheros generados:"
echo " - resultados/resultados_pruebas.txt"
echo " - resultados/resultados_experimentos_bnb.txt"
echo " - resultados/resultados_pruebas_bruteforce.txt"
echo " - resultados/resultados_pruebas_lp.txt"
echo " - resultados/resultados_experimentos_lp.txt"
