# PRACTICA 1 - ALGORITMOS VORACES

## Desde dónde ejecutar
Todos los comandos se asumen ejecutados desde la carpeta `p1` del proyecto.

```bash
cd ruta/al/proyecto/algoritmiaBasica/p1
```

## Ejecutar las herramientas de `tools`

1. (Solo la primera vez) Instalar dependencias de Python:

```bash
pip install -r tools/requirements.txt
```

2. Ejecutar el conversor (ejemplo):

```bash
python tools/conversor.py messi.png "de .png a .pgm"
python tools/conversor.py messi.pgm "de .pgm a .png"
```

Adapta los nombres de archivo de entrada y salida según lo que necesites.

## Compilar y ejecutar `code/voraz.c`

1. Compilar el programa voraz:

```bash
gcc code/voraz.c -o code/voraz
```

2. Ejecutar el programa pasando solo el **nombre** de la imagen (la ruta base está en el código):

```bash
./code/voraz messi.pgm
```

El programa se encargará de completar en todos los casos la ruta hasta `imagenes/pixel/` (tal y como está configurado en `code/voraz.c`).