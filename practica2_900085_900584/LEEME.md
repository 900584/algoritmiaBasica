# Practica 2: Programacion Dinamica - Particion de palabras

**Algoritmia Basica · Grado en Ingenieria Informatica · Curso 2025/26 · Universidad de Zaragoza**

**Autores:**
- Ibón Castarlenas Cortés — NIA: 900085
- David Puértolas Merenciano — NIA: 900584

---

## Descripcion

Dado un texto sin espacios y un diccionario de palabras, el programa determina si el texto
puede dividirse en una secuencia de palabras del diccionario y, en caso afirmativo, lista
todas las particiones posibles. Se implementan tres variantes del algoritmo:

| Variante | Estrategia | Coste (decision) |
|---|---|---|
| 1 | Recursiva pura | Exponencial |
| 2 | Recursiva con memoizacion | O(n²) |
| 3 | Iterativa con tabla (bottom-up) | O(n²) |

---

## Estructura del directorio

```
practica2_900085_900584/
├── LEEME.md
├── ejecutar.sh                     # Compila, ejecuta pruebas y experimentos
├── memoria.docx / memoria.pdf      # Informe de la practica
├── code/
│   └── separarPalabras.c           # Codigo fuente
├── tools/
│   ├── generar_pruebas.py          # Genera diccionarios y textos de prueba grandes
│   └── generar_memoria.py          # Genera el informe en .docx
└── pruebas/
    ├── diccionario_ejemplo.txt     # Diccionario del enunciado (12 palabras)
    ├── texto_megusta.txt           # Entrada: megusta
    ├── texto_megustasoldar.txt     # Entrada: megustasoldar
    ├── texto_megustadolar.txt      # Entrada: megustadolar
    ├── texto_solo.txt              # Entrada: sol
    ├── texto_helado.txt            # Entrada: helado
    ├── texto_melado.txt            # Entrada: melado
    ├── diccionario_100.txt         # Generado por generar_pruebas.py
    ├── diccionario_1000.txt        # Generado por generar_pruebas.py
    ├── diccionario_5000.txt        # Generado por generar_pruebas.py
    ├── texto_100_valido.txt        # numPal/10 palabras del diccionario de 100
    ├── texto_100_mutado.txt        # texto_100_valido con mutaciones aleatorias
    ├── texto_1000_valido.txt
    ├── texto_1000_mutado.txt
    ├── texto_5000_valido.txt
    └── texto_5000_mutado.txt
```

Los ficheros `diccionario_N.txt` y `texto_N_*.txt` se generan automaticamente
al ejecutar `bash ejecutar.sh` (o manualmente con `python3 tools/generar_pruebas.py`).

---

## Compilacion y ejecucion

### Compilacion manual

```bash
gcc -O2 -o separarPalabras code/separarPalabras.c -std=c99
```

### Uso del programa

```bash
./separarPalabras <var> <diccionario> <texto>
```

- `var`: variante del algoritmo (`1` recursiva, `2` con memoizacion, `3` tabla)
- `diccionario`: fichero con las palabras, una por linea
- `texto`: fichero con la cadena a verificar (los espacios se ignoran)

**Salida:**
- Si es posible: `Si.` seguido de la lista de particiones con formato `- 'palabra1 palabra2 ...'`
- Si no es posible: `No.`

### Ejecucion automatica (pruebas + experimentos)

```bash
bash ejecutar.sh
```

El script:
1. Compila el programa
2. Ejecuta los casos basicos del enunciado con las tres variantes
3. Genera los ficheros de prueba grandes (`generar_pruebas.py`)
4. Ejecuta las tres variantes sobre cada escenario de experimento midiendo tiempos
   (con limite de 60s por ejecucion para evitar esperas excesivas)

---

## Ficheros de prueba

### Formato

- **Diccionario**: una palabra por linea; se ignoran espacios extra y puntuacion al final
- **Texto**: la cadena a verificar; los espacios, tabulaciones y saltos de linea se eliminan

### Generacion de ficheros grandes (Tarea 3)

```bash
python3 tools/generar_pruebas.py [--seed N]
```

Para cada tamaño N ∈ {100, 1000, 5000} genera:

- **`diccionario_N.txt`**: N palabras unicas combinando silabas del español
- **`texto_N_valido.txt`**: concatenacion de `numPal/10` palabras del diccionario
  (siempre segmentable)
- **`texto_N_mutado.txt`**: mismo texto con mutaciones aleatorias letra a letra
  con probabilidad `1/(LF×10)`, donde `LF` es la longitud del texto

La opcion `--seed N` permite reproducir exactamente los mismos ficheros.

---

## Diseño del algoritmo

### Ecuacion de recurrencia

Sea `texto[0..n-1]` la cadena y `puede(i)` = 1 si el sufijo `texto[i..n-1]` es particionable:

```
puede(n) = 1                                              (caso base: cadena vacia)
puede(i) = OR { texto[i..j] en dicc  AND  puede(j+1) }   para todo j >= i
```

### Variantes

- **Variante 1**: implementacion directa de la recurrencia sin memoria. Puede recalcular
  el mismo subproblema muchas veces → coste exponencial.
- **Variante 2**: igual que la 1 pero con tabla `mem[i]` para no recalcular `puede(i)`.
  Coste de la decision: O(n²).
- **Variante 3**: rellena la tabla `posible[n..0]` de forma iterativa (sin recursion),
  usando los valores ya calculados. Coste O(n²), sin sobrecarga de pila. Ademas, usa
  `posible[]` para podar ramas muertas durante la enumeracion de particiones.

En los tres casos, la enumeracion de todas las particiones validas puede ser exponencial
si el numero de soluciones es muy grande.

### Estructura de datos del diccionario

Tabla hash con funcion djb2 y resolucion de colisiones por encadenamiento.
Permite busqueda en O(1) promedio frente a O(m) de busqueda lineal.
