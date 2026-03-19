#!/usr/bin/env python3
"""
Generador de casos de prueba para la Practica 3.
Genera grafos aleatorios conexos con pesos aleatorios.

Uso:
    python3 generar_pruebas.py [--seed N]

Genera ficheros en pruebas/ con distintas combinaciones de (n, k).
"""

import argparse
import os
import random

PRUEBAS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'pruebas')


def generar_grafo_conexo(n, densidad=0.3, peso_min=1, peso_max=50, rng=None):
    """Genera un grafo conexo con n vertices y aristas aleatorias."""
    if rng is None:
        rng = random.Random()

    aristas = {}

    # Arbol spanning aleatorio para garantizar conexion
    nodos = list(range(1, n + 1))
    rng.shuffle(nodos)
    for i in range(1, n):
        u = nodos[i - 1]
        v = nodos[i]
        if u > v:
            u, v = v, u
        peso = rng.randint(peso_min, peso_max)
        aristas[(u, v)] = peso

    # Aristas adicionales segun densidad
    max_aristas = n * (n - 1) // 2
    num_extra = int(max_aristas * densidad) - (n - 1)
    if num_extra > 0:
        posibles = []
        for i in range(1, n + 1):
            for j in range(i + 1, n + 1):
                if (i, j) not in aristas:
                    posibles.append((i, j))
        rng.shuffle(posibles)
        for idx in range(min(num_extra, len(posibles))):
            u, v = posibles[idx]
            peso = rng.randint(peso_min, peso_max)
            aristas[(u, v)] = peso

    return aristas


def escribir_caso(f, n, aristas, c_existentes, k):
    """Escribe un caso de prueba en el fichero f."""
    m = len(aristas)
    f.write(f"{n} {m} {len(c_existentes)} {k}\n")
    for (u, v), peso in sorted(aristas.items()):
        f.write(f"{u} {v} {peso}\n")
    f.write(" ".join(str(x) for x in sorted(c_existentes)) + "\n")


def generar_fichero(nombre, casos, rng):
    """Genera un fichero con multiples casos de prueba."""
    ruta = os.path.join(PRUEBAS_DIR, nombre)
    with open(ruta, 'w') as f:
        f.write(f"{len(casos)}\n")
        for n, c_count, k, densidad in casos:
            aristas = generar_grafo_conexo(n, densidad=densidad, rng=rng)
            # Elegir c centros existentes aleatorios
            nodos = list(range(1, n + 1))
            rng.shuffle(nodos)
            existentes = sorted(nodos[:c_count])
            escribir_caso(f, n, aristas, existentes, k)
    print(f"  Generado: {ruta}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--seed', type=int, default=42)
    args = parser.parse_args()

    rng = random.Random(args.seed)
    os.makedirs(PRUEBAS_DIR, exist_ok=True)

    print("Generando ficheros de prueba para experimentacion...")

    # Experimento 1: Variar n con k fijo
    # (n, c, k, densidad)
    generar_fichero("exp_variar_n.txt", [
        (8, 1, 2, 0.4),
        (10, 1, 2, 0.4),
        (12, 1, 2, 0.35),
        (15, 2, 2, 0.3),
        (18, 2, 2, 0.25),
        (20, 2, 2, 0.25),
        (25, 2, 2, 0.2),
    ], rng)

    # Experimento 2: Variar k con n fijo
    generar_fichero("exp_variar_k.txt", [
        (15, 2, 1, 0.3),
        (15, 2, 2, 0.3),
        (15, 2, 3, 0.3),
        (15, 2, 4, 0.3),
        (15, 2, 5, 0.3),
        (15, 2, 6, 0.3),
    ], rng)

    # Experimento 3: Casos mas grandes (para ver limites)
    generar_fichero("exp_grande.txt", [
        (30, 3, 3, 0.2),
        (40, 3, 3, 0.15),
        (50, 4, 3, 0.12),
    ], rng)

    print("Generacion completada.")


if __name__ == '__main__':
    main()
