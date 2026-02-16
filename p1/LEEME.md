# Práctica 1: Algoritmos Voraces - String Art

## Descripción General

Este proyecto implementa un algoritmo voraz para generar diseños de "string art" (hilorama) a partir de imágenes en escala de grises. El algoritmo selecciona iterativamente hilos entre pares de clavos dispuestos en círculo alrededor de la imagen para aproximar la imagen original.

## Estructura del Directorio

```
p1/
├── code/
│   └── voraz.c          # Programa principal con el algoritmo voraz
├── tools/
│   ├── conversor.py     # Herramienta para convertir PNG ↔ PGM
│   └── requirements.txt  # Dependencias de Python (opencv-python)
├── imagenes/
│   ├── raw/             # Imágenes originales en formato PNG
│   └── pixel/           # Imágenes convertidas a formato PGM
├── pruebas/             # Casos de prueba (actualmente vacío)
├── resultados/          # Imágenes resultado generadas (se crea al ejecutar)
├── ejecutar.sh          # Script de compilación y ejecución
└── LEEME.md            # Este archivo

```

## Requisitos

- Compilador GCC con soporte C99
- Biblioteca matemática estándar (libm)
- Python 3 (opcional, solo para conversor.py)
- OpenCV para Python (opcional, solo para conversor.py)

## Compilación

Para compilar el programa manualmente:

```bash
gcc -o hilos code/voraz.c -lm -std=c99
```

El ejecutable resultante se llamará `hilos`.

## Ejecución

### Forma de uso

```bash
./hilos <n> <p> <s> <imagen_entrada> <imagen_salida>
```

**Parámetros:**
- `n`: Número de clavos que se utilizarán (ej: 1000)
- `p`: Número de hilos candidatos a evaluar en cada iteración (ej: 1100)
- `s`: Número de mejores hilos a seleccionar en cada paso (ej: 30)
- `imagen_entrada`: Ruta al archivo PGM de entrada
- `imagen_salida`: Ruta donde se guardará el archivo PGM resultado

**Ejemplo:**
```bash
./hilos 1000 1100 30 imagenes/pixel/messi.pgm resultado.pgm
```

### Ejecución automática con script

Para ejecutar todos los casos de prueba automáticamente:

```bash
./ejecutar.sh
```

Este script:
1. Compila el programa
2. Crea el directorio `resultados/` si no existe
3. Ejecuta varios casos de prueba con diferentes parámetros
4. Guarda los resultados en `resultados/`

## Algoritmo Implementado

### Descripción General

El algoritmo sigue estos pasos iterativos:

1. **Generación de clavos**: Se generan `n` clavos distribuidos uniformemente en un círculo alrededor de la imagen
2. **Selección de candidatos**: En cada iteración se seleccionan `p` hilos candidatos aleatoriamente
3. **Evaluación**: Se calcula el error que reduciría cada hilo (suma de valores de gris que atraviesa)
4. **Selección voraz**: Se ordenan los candidatos y se seleccionan los `s` mejores
5. **Dibujo**: Se dibujan los hilos seleccionados usando el algoritmo de Bresenham
6. **Actualización**: Se actualiza la imagen de trabajo restando los valores de los píxeles atravesados
7. **Parada**: El algoritmo se detiene cuando:
   - Se alcanza un número máximo de hilos (10000)
   - El error no mejora en 10 iteraciones consecutivas
   - El error alcanza un umbral mínimo (1000.0)

### Decisiones de Diseño

1. **Heurística de selección**: Se utiliza selección aleatoria de parejas de clavos para generar candidatos
2. **Función de error**: Suma de los valores de gris de los píxeles que atraviesa cada hilo
3. **Dibujo de líneas**: Implementación del algoritmo de Bresenham según el apéndice del enunciado
4. **Condiciones de parada**: Combinación de límite de iteraciones, estabilización del error y umbral mínimo
5. **Hilos**: Se asumen opacos y negros (valor 0 en escala de grises)

### Estructura de Datos

- **Imagen**: Matriz bidimensional de píxeles en escala de grises (0-255)
- **Punto**: Coordenadas (x, y) de un clavo
- **Hilo**: Par de índices de clavos con su error asociado
- **Parámetros**: Valores n, p, s del programa

## Salida del Programa

El programa genera:

1. **Imagen resultado**: Archivo PGM con el diseño de hilos generado
2. **Estadísticas en consola**:
   - Parámetros utilizados (n, p, s)
   - Número de hilos dibujados
   - Error final (suma de cuadrados de diferencias)
   - Tiempo de ejecución en segundos

## Conversión de Imágenes

Para convertir imágenes PNG a PGM (formato requerido):

```bash
python tools/conversor.py nombre_imagen.png
```

Esto convierte `imagenes/raw/nombre_imagen.png` a `imagenes/pixel/nombre_imagen.pgm`

Para convertir PGM a PNG:

```bash
python tools/conversor.py nombre_imagen.pgm
```

## Casos de Prueba

El script `ejecutar.sh` incluye tres casos de prueba:

1. **Caso 1**: `messi.pgm` con parámetros pequeños (n=100, p=200, s=10)
2. **Caso 2**: `ilita-topuria.pgm` con parámetros medianos (n=200, p=400, s=20)
3. **Caso 3**: `messi.pgm` con parámetros del enunciado (n=1000, p=1100, s=30)

## Limitaciones y Mejoras Futuras

### Limitaciones Actuales

- Los hilos son siempre negros (valor 0) y opacos
- No se implementa mapa de importancia
- La selección de candidatos es completamente aleatoria

### Mejoras Sugeridas (No Implementadas)

1. **Brillo, color y transparencia**: Permitir hilos con diferentes valores de gris y opacidad
2. **Mapa de importancia**: Priorizar ciertas áreas de la imagen
3. **Optimización de selección**: Mejorar la estrategia de selección de candidatos

## Notas Técnicas

- El algoritmo utiliza números aleatorios, por lo que resultados pueden variar entre ejecuciones
- Para imágenes grandes o muchos clavos, el tiempo de ejecución puede ser considerable
- El formato PGM utilizado es P2 (ASCII), compatible con la mayoría de visualizadores

## Autores

[Indicar nombres y NIAs de los miembros del grupo]

## Referencias

- [DLD22] Baptiste Demoussel, Caroline Larboulette, and Ravi Dattatreya. A Greedy Algorithm for Generative String Art. In Bridges 2022 Conference Proceedings, Aalto, Finland, August 2022.
