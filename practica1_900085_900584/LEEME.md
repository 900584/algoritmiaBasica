# Practica 1: Algoritmos Voraces - String Art

## Autores

- David Puértolas Merenciano — NIA: 900584
- Ibón Castarlenas Cortés — NIA: 900085

---

## Descripcion general

Este proyecto implementa un algoritmo voraz para generar diseños de "string art"
(hilorama) a partir de imagenes en escala de grises. El algoritmo selecciona
iterativamente hilos entre pares de clavos dispuestos en el borde de la imagen
para aproximar la imagen original.

---

## Estructura del directorio

```
p1/
├── LEEME.md               # Este archivo
├── memoria.pdf            # Informe con diseño, analisis y resultados
├── ejecutar.sh            # Script de compilacion y ejecucion de pruebas
├── code/
│   └── hilos.c            # Codigo fuente del programa principal
├── pruebas/               # Imagenes de entrada para los casos de prueba
│   ├── messi.pgm
│   ├── iliaTopuria.pgm
│   ├── emoji.pgm
│   └── qr.pgm
└── tools/
    ├── conversor.py       # Herramienta para convertir PNG <-> PGM
    ├── experimentos.sh    # Script de experimentacion (Tarea 3)
    └── requirements.txt   # Dependencias Python para el conversor

```

---

## Compilacion

El programa se compila automaticamente al ejecutar `ejecutar.sh`. Para compilar
manualmente:

```bash
gcc -O2 -o hilos code/hilos.c -lm -std=c99
```

El ejecutable resultante se llama `hilos`.

---

## Ejecucion

### Forma de uso

```bash
./hilos <n> <p> <s> <imagen_entrada> <imagen_salida>
```

**Parametros:**
- `n`: Numero de clavos distribuidos en el borde de la imagen (ej: 1000)
- `p`: Numero de hilos candidatos evaluados en cada iteracion (ej: 1100)
- `s`: Numero de mejores hilos seleccionados en cada paso (ej: 30)
- `imagen_entrada`: Ruta al archivo PGM de entrada
- `imagen_salida`: Ruta donde se guardara el archivo PGM resultado

**Ejemplo del enunciado:**
```bash
./hilos 1000 1100 30 pruebas/messi.pgm resultados/messi_resultado.pgm
```

### Ejecucion automatica con script

```bash
bash ejecutar.sh
```

Este script compila el programa y ejecuta 5 casos de prueba con las imagenes
de `pruebas/`, guardando los resultados en `resultados/`.

---

## Algoritmo implementado

### Descripcion general

El algoritmo sigue estos pasos iterativos:

1. **Generacion de clavos**: Se generan `n` clavos distribuidos uniformemente
   en el perimetro rectangular de la imagen.
2. **Seleccion de candidatos**: En cada iteracion se seleccionan `p` pares de
   clavos aleatorios como hilos candidatos.
3. **Evaluacion**: Se calcula el beneficio de cada hilo (suma de oscuridad
   faltante en los pixeles que atraviesa, usando Bresenham).
4. **Seleccion voraz**: Se ordenan los candidatos y se seleccionan los `s` mejores.
5. **Dibujo**: Se dibujan los hilos seleccionados sobre la imagen resultado.
6. **Actualizacion**: Se resta la oscuridad aportada en la imagen de trabajo.
7. **Parada**: El algoritmo se detiene cuando no hay mejora en 20 iteraciones
   consecutivas o se alcanzan 50000 hilos.

### Decisiones de diseño

- **Heuristica de seleccion**: Parejas de clavos elegidas aleatoriamente.
- **Funcion de beneficio**: Suma de oscuridad faltante (diferencia entre imagen
  original e imagen actual) a lo largo del trayecto del hilo.
- **Trazado de lineas**: Algoritmo de Bresenham segun el apendice del enunciado.
- **Clavos**: Distribuidos en el perimetro rectangular (no circular).
- **Hilos**: Opacos y negros (valor 0 en escala de grises).

---

## Salida del programa

El programa genera:

1. **Imagen resultado** en formato PGM.
2. **Estadisticas por consola**:
   - Parametros utilizados (n, p, s)
   - Numero de hilos dibujados
   - Error final (suma de cuadrados de diferencias con la imagen original)
   - Tiempo de ejecucion en segundos

---

## Repeticion de experimentos (Tarea 3)

Para reproducir todos los experimentos de la memoria:

```bash
bash tools/experimentos.sh
```

Esto genera:
- `experimentos/resultados.csv` — tabla con todas las mediciones
- `experimentos/imagenes/` — imagenes PGM de cada experimento

Los experimentos cubren:
- **EXP1**: Variacion de `n` (50, 100, 200, 400, 600, 1000)
- **EXP2**: Variacion de `p` (50, 100, 300, 600, 1000, 2000)
- **EXP3**: Variacion de `s` (5, 10, 20, 50, 100, 200)
- **EXP4**: Diversas imagenes con parametros fijos (n=200, p=500, s=20)
- **EXP5**: Parametros del enunciado (n=1000, p=1100, s=30) en todas las imagenes

---

## Conversion de imagenes (opcional)

Para convertir imagenes PNG a formato PGM (requiere Python 3 y OpenCV):

```bash
# Instalar dependencias (solo la primera vez)
pip install -r tools/requirements.txt

# Convertir PNG -> PGM
python tools/conversor.py ruta_imagen.png

# Convertir PGM -> PNG
python tools/conversor.py ruta_imagen.pgm
```

Los archivos convertidos se guardan automaticamente en:
- `conversiones/pgm/` — cuando conviertes de PNG a PGM
- `conversiones/png/` — cuando conviertes de PGM a PNG

Las carpetas se crean solas si no existen.
