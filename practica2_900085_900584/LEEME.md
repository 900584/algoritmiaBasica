# Practica 2: Programacion Dinamica - Particion de palabras

## Autores

- Ibón Castarlenas Cortés — NIA: 900085
- David Puértolas Merenciano — NIA: 900584

---

## Descripcion general

Este proyecto implementa un algoritmo de **programacion dinamica** para resolver el problema de la **particion de palabras**: determinar si una cadena de entrada puede dividirse en una secuencia de palabras de un diccionario, y en caso afirmativo, listar todas las particiones posibles.

Se implementan **tres variantes** del algoritmo:
1. **Variante 1**: Solucion recursiva pura
2. **Variante 2**: Solucion recursiva con memoria (memoizacion)
3. **Variante 3**: Solucion iterativa con tabla (bottom-up)

---

## Estructura del directorio

```
practica2_900085_900584/
├── LEEME.md                    # Este archivo
├── ejecutar.sh                 # Script de compilacion y ejecucion
├── code/
│   └── separarPalabras.c       # Codigo fuente del programa
└── pruebas/
    ├── diccionario_ejemplo.txt # Diccionario del enunciado
    ├── texto_megusta.txt       # Entrada: megusta
    ├── texto_megustasoldar.txt # Entrada: megustasoldar
    ├── texto_megustadolar.txt  # Entrada: megustadolar
    ├── texto_solo.txt          # Entrada: sol
    └── texto_helado.txt        # Entrada: helado
```

---

## Compilacion

El programa se compila automaticamente al ejecutar `ejecutar.sh`. Para compilar manualmente:

```bash
gcc -O2 -o separarPalabras code/separarPalabras.c -std=c99
```

El ejecutable resultante se llama `separarPalabras`.

---

## Ejecucion

### Forma de uso

```bash
./separarPalabras <var> <diccionario> <texto>
```

**Parametros:**
- `var`: Variante del algoritmo (1, 2 o 3)
  - **1**: Solucion recursiva pura
  - **2**: Solucion recursiva con memoizacion
  - **3**: Solucion iterativa con tabla
- `diccionario`: Ruta al fichero con las palabras del diccionario (una por linea)
- `texto`: Ruta al fichero con la cadena a verificar (sin espacios)

**Ejemplos:**
```bash
./separarPalabras 1 pruebas/diccionario_ejemplo.txt pruebas/texto_megusta.txt
./separarPalabras 2 pruebas/diccionario_ejemplo.txt pruebas/texto_megustasoldar.txt
./separarPalabras 3 pruebas/diccionario_ejemplo.txt pruebas/texto_megustadolar.txt
```

### Ejecucion automatica con script

```bash
bash ejecutar.sh
```

Este script compila el programa y ejecuta todos los casos de prueba con las tres variantes donde procede.

---

## Formato de los ficheros

### Diccionario
- Fichero de texto con una palabra por linea
- Se ignoran espacios en blanco extra
- Ejemplo:
  ```
  me
  gusta
  sol
  dar
  soldar
  ```

### Texto de entrada
- Fichero con la cadena a verificar
- Se eliminan espacios, saltos de linea y tabulaciones
- Ejemplo: el contenido `megustasoldar` representa la cadena sin espacios

---

## Salida del programa

- **Si se puede particionar**: imprime `Si.` seguido de las particiones, cada una con formato `  • 'palabra1 palabra2 ...'`
- **Si no se puede**: imprime `No.`

**Ejemplo de salida (megustasoldar):**
```
Si.
  • 'me gusta soldar'
  • 'me gusta sol dar'
```

---

## Diseño del algoritmo

### Ecuacion de recurrencia

Sea `texto[0..n-1]` la cadena de entrada y `puede(i)` indicar si el sufijo `texto[i..n-1]` puede particionarse:

- **Caso base**: `puede(n) = true` (cadena vacia)
- **Recurrencia**: `puede(i) = OR_{j} (texto[i..j] en dicc AND puede(j+1))` para todo j tal que el prefijo este en el diccionario

### Estructura de datos del diccionario

Se utiliza una **tabla hash** (funcion djb2) para permitir busqueda de palabras en tiempo O(1) promedio, en lugar de O(m) con busqueda lineal en un array de m palabras.

### Reduccion del tiempo de ejecucion

- **Variante 1**: Complejidad exponencial por solapamiento de subproblemas
- **Variante 2**: Memoizacion evita recalcular `puede(i)`, reduciendo a O(n^2) llamadas
- **Variante 3**: Tabla rellena de forma ordenada (desde n hacia 0), coste O(n^2) en tiempo y O(n) en espacio adicional

---

## Repeticion de las pruebas

Para que los profesores puedan repetir las pruebas:

1. Descomprimir `practica2.zip` (o el nombre del directorio entregado)
2. Navegar al directorio de la practica
3. Ejecutar: `bash ejecutar.sh`

El script compila y ejecuta automaticamente todos los casos de prueba definidos.

---

## Notas adicionales

- El programa debe ejecutarse en el entorno **lab000** (Linux)
- No hay menus interactivos; todo se controla mediante argumentos de linea de comandos
- Los ficheros de prueba incluyen los ejemplos del enunciado de la practica
