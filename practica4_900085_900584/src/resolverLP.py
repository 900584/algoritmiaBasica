#!/usr/bin/env python3
"""
resolverLP.py  -  Programacion lineal entera (ILP) para el problema
de formacion de equipos con minimo conflicto total.

Uso:
    python3 resolverLP.py <fichero_entrada> <fichero_salida>

Requiere:
    pip install pulp

Formato de entrada/salida: identico al de formarEquipos.c
"""

import sys
import time
from itertools import combinations

try:
    import pulp
except ImportError:
    print(
        "Error: se necesita la libreria 'pulp'.\n"
        "Instalala con:  pip install pulp",
        file=sys.stderr,
    )
    sys.exit(1)


def team_cost(conflicts, i, j, k):
    """Conflicto total dirigido de un equipo {i, j, k}."""
    return (
        conflicts[i][j] + conflicts[i][k]
        + conflicts[j][i] + conflicts[j][k]
        + conflicts[k][i] + conflicts[k][j]
    )


def solve_case_lp(out, case_id, n, conflicts):
    """Formula y resuelve un caso como ILP con PuLP/CBC."""
    if n <= 0 or n % 3 != 0:
        out.write(f"Caso {case_id}\n")
        out.write(f"N={n} invalido. Se requiere N > 0 y N mod 3 = 0.\n\n")
        return

    t0 = time.perf_counter()

    # --- 1. Todas las ternas candidatas (i < j < k) ---
    triples = list(combinations(range(n), 3))
    c = {t: team_cost(conflicts, *t) for t in triples}

    # --- 2. Modelo ILP ---
    prob = pulp.LpProblem(f"FormarEquipos_C{case_id}", pulp.LpMinimize)

    # Variable binaria: x[t] = 1 si la terna t forma un equipo
    x = {t: pulp.LpVariable(f"x_{t[0]}_{t[1]}_{t[2]}", cat="Binary") for t in triples}

    # Funcion objetivo: minimizar conflicto total
    prob += pulp.lpSum(c[t] * x[t] for t in triples), "conflicto_total"

    # Restriccion: cada participante pertenece a exactamente un equipo
    for p in range(n):
        in_triples = [t for t in triples if p in t]
        prob += pulp.lpSum(x[t] for t in in_triples) == 1, f"part_{p}"

    # --- 3. Resolver con CBC (sin output de consola) ---
    solver = pulp.PULP_CBC_CMD(msg=0)
    status = prob.solve(solver)

    t1 = time.perf_counter()
    elapsed_ms = (t1 - t0) * 1000.0

    optimal_value = int(round(pulp.value(prob.objective)))
    n_vars = len(triples)

    out.write(f"Caso {case_id}\n")
    out.write(f"N={n}\n")
    out.write(f"Tiempo_ms={elapsed_ms:.3f}\n")
    out.write(f"Variables_binarias={n_vars}\n")
    out.write(f"Estado={pulp.LpStatus[status]}\n")
    out.write(f"Valor_optimo={optimal_value}\n")

    selected = [t for t in triples if (pulp.value(x[t]) or 0.0) > 0.5]
    out.write("Equipos_optimos (indices 1..N):\n")
    for idx, (a, b, cc) in enumerate(selected, 1):
        out.write(f"  Equipo_{idx}=({a+1},{b+1},{cc+1})\n")
    out.write("\n")


def read_cases(path):
    """Lee todos los bloques del fichero de entrada y devuelve lista de (n, conflicts)."""
    with open(path) as f:
        tokens = f.read().split()

    cases = []
    pos = 0
    while pos < len(tokens):
        n = int(tokens[pos])
        pos += 1
        conflicts = []
        for _ in range(n):
            row = [int(tokens[pos + j]) for j in range(n)]
            conflicts.append(row)
            pos += n
        cases.append((n, conflicts))
    return cases


def main():
    if len(sys.argv) != 3:
        print(f"Uso: {sys.argv[0]} <fichero_entrada> <fichero_salida>", file=sys.stderr)
        sys.exit(1)

    in_path, out_path = sys.argv[1], sys.argv[2]

    try:
        cases = read_cases(in_path)
    except OSError as e:
        print(f"Error: no se puede abrir {in_path}: {e}", file=sys.stderr)
        sys.exit(1)

    with open(out_path, "w") as out:
        for case_id, (n, conflicts) in enumerate(cases, 1):
            solve_case_lp(out, case_id, n, conflicts)


if __name__ == "__main__":
    main()
