#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
generar_pruebas.py - Generador de diccionarios y textos de prueba
Practica 2: Programacion Dinamica - Particion de palabras

Algoritmia Basica - Grado en Ingenieria Informatica - Curso 2025/26
Universidad de Zaragoza

Uso:
    python3 tools/generar_pruebas.py [--seed N]

Genera en pruebas/:
  - diccionario_100.txt, diccionario_1000.txt, diccionario_5000.txt
  - texto_<N>_valido.txt   : numPal/10 palabras aleatorias del diccionario concatenadas
  - texto_<N>_mutado.txt   : mismo texto con mutaciones aleatorias letra a letra
                             (probabilidad 1/(LF*10) por caracter, LF = longitud del texto)
"""

import random
import os
import sys
import argparse

# ---------------------------------------------------------------------------
# SILABAS del español para construir palabras sinteticas
# Se combinan aleatoriamente para formar palabras de longitud variable
# ---------------------------------------------------------------------------
SILABAS = [
    "a", "e", "i", "o", "u",
    "ba", "be", "bi", "bo", "bu",
    "ca", "co", "cu", "que", "qui",
    "da", "de", "di", "do", "du",
    "fa", "fe", "fi", "fo", "fu",
    "ga", "ge", "go", "gu",
    "la", "le", "li", "lo", "lu",
    "ma", "me", "mi", "mo", "mu",
    "na", "ne", "ni", "no", "nu",
    "pa", "pe", "pi", "po", "pu",
    "ra", "re", "ri", "ro", "ru",
    "sa", "se", "si", "so", "su",
    "ta", "te", "ti", "to", "tu",
    "va", "ve", "vi", "vo",
    "bra", "bre", "bri", "bro",
    "tra", "tre", "tri", "tro",
    "pra", "pre", "pri", "pro",
    "cla", "cle", "clo", "clu",
    "fla", "fle", "flo", "flu",
    "gra", "gre", "gri", "gro",
    "al", "el", "in", "es",
    "ar", "er", "or",
    "an", "en", "on",
    "cion", "sion", "dad", "mente",
    "ado", "ada", "ero", "era",
    "ista", "ismo", "ura", "aje",
]


def generar_palabra(min_sil=1, max_sil=4):
    """Genera una palabra aleatoria combinando silabas."""
    n = random.randint(min_sil, max_sil)
    return "".join(random.choice(SILABAS) for _ in range(n))


def generar_diccionario(n, seed=42):
    """
    Genera un conjunto de n palabras unicas.
    Retorna una lista ordenada.
    """
    random.seed(seed)
    palabras = set()
    intentos = 0
    while len(palabras) < n and intentos < n * 50:
        p = generar_palabra()
        if len(p) >= 2:
            palabras.add(p)
        intentos += 1
    if len(palabras) < n:
        print(f"  AVISO: solo se pudieron generar {len(palabras)} palabras unicas (pedidas: {n})")
    return sorted(palabras)


def generar_texto_valido(palabras_dicc, seed=7):
    """
    Selecciona numPal/10 palabras aleatorias del diccionario y las concatena.
    Devuelve (cadena_sin_espacios, lista_de_palabras_seleccionadas).
    """
    random.seed(seed)
    num_pal = max(1, len(palabras_dicc) // 10)
    seleccion = random.choices(palabras_dicc, k=num_pal)
    return "".join(seleccion), seleccion


def mutar_texto(texto, seed=13):
    """
    Aplica mutaciones aleatorias al texto.
    Cada letra tiene probabilidad 1/(LF*10) de ser cambiada por otra letra aleatoria.
    LF = longitud en caracteres del texto.
    """
    random.seed(seed)
    lf = len(texto)
    if lf == 0:
        return texto
    prob = 1.0 / (lf * 10)
    letras = "abcdefghijklmnopqrstuvwxyz"
    resultado = []
    mutaciones = 0
    for c in texto:
        if random.random() < prob:
            nueva = random.choice(letras)
            resultado.append(nueva)
            mutaciones += 1
        else:
            resultado.append(c)
    return "".join(resultado), mutaciones, prob


def guardar_lineas(lineas, ruta):
    with open(ruta, "w", encoding="utf-8") as f:
        for l in lineas:
            f.write(l + "\n")


def guardar_texto(texto, ruta):
    with open(ruta, "w", encoding="utf-8") as f:
        f.write(texto + "\n")


def main():
    parser = argparse.ArgumentParser(
        description="Genera diccionarios y textos de prueba para separarPalabras"
    )
    parser.add_argument("--seed", type=int, default=42,
                        help="Semilla aleatoria para reproducibilidad (default: 42)")
    args = parser.parse_args()

    # Directorio de pruebas (relativo a la ubicacion de este script)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    pruebas_dir = os.path.join(script_dir, "..", "pruebas")
    os.makedirs(pruebas_dir, exist_ok=True)

    tamanos = [100, 1000, 5000]

    print("=" * 60)
    print("Generador de pruebas - Practica 2 Algoritmia Basica")
    print("=" * 60)

    for n in tamanos:
        print(f"\n--- Diccionario de {n} palabras ---")

        # Generar y guardar diccionario
        palabras = generar_diccionario(n, seed=args.seed)
        ruta_dicc = os.path.join(pruebas_dir, f"diccionario_{n}.txt")
        guardar_lineas(palabras, ruta_dicc)
        print(f"  Diccionario : {ruta_dicc}  ({len(palabras)} palabras)")

        # Texto valido (sin modificar)
        texto, seleccion = generar_texto_valido(palabras, seed=args.seed + 1)
        ruta_valido = os.path.join(pruebas_dir, f"texto_{n}_valido.txt")
        guardar_texto(texto, ruta_valido)
        muestra = " ".join(seleccion[:5]) + ("..." if len(seleccion) > 5 else "")
        print(f"  Texto valido: {ruta_valido}")
        print(f"    {len(seleccion)} palabras seleccionadas, {len(texto)} caracteres")
        print(f"    Muestra: '{muestra}'")

        # Texto mutado
        texto_mutado, n_mut, prob = mutar_texto(texto, seed=args.seed + 2)
        ruta_mutado = os.path.join(pruebas_dir, f"texto_{n}_mutado.txt")
        guardar_texto(texto_mutado, ruta_mutado)
        print(f"  Texto mutado: {ruta_mutado}")
        print(f"    Prob. mutacion por letra: 1/({len(texto)}*10) = {prob:.7f}")
        print(f"    Letras mutadas: {n_mut} de {len(texto)}")

    print("\n" + "=" * 60)
    print("Ficheros generados correctamente en pruebas/")
    print("=" * 60)


if __name__ == "__main__":
    main()

