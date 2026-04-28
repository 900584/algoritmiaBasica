# Practica 4 - Ramificacion y poda

**Asignatura:** Algoritmia Basica  
**Autores:** David Puertolas Merenciano (900584), Ibon Castarlenas Cortes (90085)

## 1. Diseno del algoritmo

### 1.1 Representacion de soluciones y arbol de busqueda

Cada solucion es una particion de los `N` participantes en `N/3` equipos de 3 personas.

La representacion de un nodo parcial `x` del arbol:

- `x.used_mask`: participantes ya asignados.
- `x.teams[0..k-1]`: equipos cerrados construidos hasta el momento.
- `x.cost_so_far`: conflicto acumulado de esos `k` equipos.

**Ramificacion:** en cada nodo se toma el menor indice `i` no asignado y se generan hijos combinandolo con cada pareja `(j,k)` no asignada con `i < j < k`.  
Cada hijo anade exactamente un equipo `(i,j,k)`.

**Profundidad:** `N/3` niveles (uno por equipo cerrado).  
**Tamano del espacio de busqueda (sin poda):**

\[
\frac{N!}{(3!)^{N/3}\,(N/3)!}
\]

que corresponde al numero de particiones en ternas sin orden interno ni entre ternas.

### 1.2 Funcion de coste \(c(x)\)

Conflicto de un equipo \(T=\{a,b,c\}\):

\[
\text{coste}(T)=c_{ab}+c_{ac}+c_{ba}+c_{bc}+c_{ca}+c_{cb}
\]

Para un nodo hoja (solucion completa), \(c(x)\) es la suma de costes de todos sus equipos.  
El objetivo es minimizar \(c(x)\).

### 1.3 Funcion de coste estimado \(\hat{c}(x)\)

Se usa una cota inferior admisible:

\[
\hat{c}(x)=\text{cost\_so\_far}(x)+LB(x)
\]

donde \(LB(x)\) se calcula sobre participantes no asignados.  
Para cada participante libre \(p\), se toman sus dos conflictos salientes minimos hacia otros participantes libres:

\[
m_1(p)+m_2(p)
\]

Entonces:

\[
LB(x)=\sum_{p\ \text{libre}} (m_1(p)+m_2(p))
\]

Esta cota es admisible porque en cualquier completado cada participante aporta exactamente dos arcos salientes hacia sus companeros de equipo, y esa eleccion no puede ser menor que la suma de sus dos salientes minimos disponibles.

### 1.4 Funcion de poda \(U\)

Se mantiene una **cota superior global** \(U\) (mejor solucion factible conocida).

- Inicializacion: completado voraz desde la raiz (heuristica).
- Actualizacion: cada vez que se encuentra una solucion factible mejor.

**Regla de poda:** si \(\hat{c}(x)\ge U\), el nodo `x` se descarta.

Adicionalmente, se poda tambien si `cost_so_far >= U` (costes no negativos).

### 1.5 Estrategia de exploracion

Se usa una cola de prioridad (min-heap) de nodos vivos, priorizada por \(\hat{c}(x)\) (estrategia de minimo coste estimado, siguiendo el enfoque de clase).

---

## 2. Implementacion

### 2.1 Organizacion del codigo

- `src/formarEquipos.c`: implementa Branch&Bound.
  - Lectura de casos.
  - Estructura de nodo.
  - Cola de prioridad minima.
  - Cota inferior `LB`.
  - Heuristica voraz para `U`.
  - Escritura de resultados por caso.
- `src/verificar_bruteforce.c`: solucion exacta por fuerza bruta para instancias pequenas, usada para validar la correccion del programa principal.

### 2.2 Tipos de datos principales

- `Team`: terna de participantes.
- `Node`:
  - mascara de asignados,
  - numero de equipos cerrados,
  - coste acumulado,
  - coste estimado,
  - vector de equipos.
- `MinHeap`: cola de prioridad de punteros a nodos.

### 2.3 Entrada y salida

**Entrada:** bloques consecutivos:

1. entero `N` (`N mod 3 = 0`);
2. matriz `N x N` de conflictos dirigidos.

**Salida (por caso):**

- tiempo de ejecucion en ms,
- nodos generados,
- valor optimo,
- equipos de una solucion optima (extra informativo).

---

## 3. Experimentacion

### 3.1 Enfoque de pruebas

Se han preparado dos tipos de ficheros:

- `pruebas/pruebas.txt`: validacion funcional.
  - Caso pequeno del enunciado (`N=6`).
  - Caso adicional (`N=9`).
- `pruebas/experimentos.txt`: analisis de prestaciones con instancias grandes.
  - 3 casos con `N=12`.
  - 3 casos con `N=15`.
  - 5 casos con `N=18`.

### 3.2 Verificacion de correccion

Para `pruebas/pruebas.txt` se compara Branch&Bound con fuerza bruta:

- mismo valor optimo esperado en ambos metodos.

Automatizado en `ejecutar.sh`.

### 3.3 Metricas analizadas

- **Tiempo de ejecucion** (ms).
- **Numero de nodos generados**.

### 3.4 Resultados representativos observados

Tras ejecutar el programa con `pruebas/pruebas.txt` y `pruebas/experimentos.txt`, se obtienen los siguientes rangos:

| N  | Tiempo (ms) aprox. | Nodos generados aprox. |
|----|---------------------|------------------------|
| 6  | 0                   | 11                     |
| 9  | 0                   | 259                    |
| 12 | 0-1                 | 1.898-3.786            |
| 15 | 8-19                | 33.257-76.109          |
| 18 | 88-823              | 305.829-3.174.065      |

### 3.5 Analisis

- El crecimiento de nodos y tiempo con `N` es muy acusado, como era esperable por la naturaleza combinatoria del problema.
- Aun asi, la poda y la estimacion reducen de forma importante la exploracion respecto al espacio completo.
- La cota voraz inicial ayuda a podar desde el inicio; cuanto mas ajustada sale, mejor rendimiento.
- Hay variabilidad entre instancias del mismo `N`: depende mucho de la estructura concreta de la matriz de conflictos.

---

## 4. (Tarea 4) Enfoque con programacion lineal

### 4.1 Formalizacion como problema de programacion lineal entera (ILP)

**Variables de decision.** Para cada terna ordenada \(i < j < k\) con \(i,j,k \in \{0,\ldots,N-1\}\):

\[
x_{ijk} \in \{0,1\}, \quad x_{ijk}=1 \iff \text{los participantes } i,j,k \text{ forman un equipo.}
\]

El numero total de variables binarias es \(\binom{N}{3}\).

**Funcion objetivo.** Minimizar el conflicto total:

\[
\min \sum_{i<j<k} \bigl(c_{ij}+c_{ik}+c_{ji}+c_{jk}+c_{ki}+c_{kj}\bigr)\,x_{ijk}
\]

**Restricciones de cobertura exacta.** Cada participante \(p\) pertenece a exactamente un equipo:

\[
\sum_{\substack{i<j<k \\ p\,\in\,\{i,j,k\}}} x_{ijk} = 1 \quad \forall p \in \{0,\ldots,N-1\}
\]

El modelo tiene \(\binom{N}{3}\) variables y \(N\) restricciones de igualdad (ademas de la integralidad).

### 4.2 Implementacion

Se utiliza la libreria Python **PuLP** (version libre, incluye el solver **CBC** de COIN-OR):

```
pip install pulp
```

El programa `src/resolverLP.py` lee el mismo formato de entrada que `formarEquipos.c` y escribe el mismo formato de salida. Por cada caso de prueba:

1. Genera todas las ternas \(\binom{N}{3}\) y calcula su coste.
2. Construye el modelo ILP con PuLP.
3. Lo resuelve con CBC (sin mensajes de consola).
4. Escribe tiempo, numero de variables, estado, valor optimo y equipos.

Ejecucion:

```
python3 src/resolverLP.py pruebas/pruebas.txt resultados/resultados_pruebas_lp.txt
python3 src/resolverLP.py pruebas/experimentos.txt resultados/resultados_experimentos_lp.txt
```

### 4.3 Validacion y comparacion de resultados

Los valores optimos obtenidos por ILP y por Branch\&Bound coinciden en **todos los casos de prueba**, lo que valida la correctitud de ambas implementaciones.

**Casos de validacion funcional** (`pruebas/pruebas.txt`):

| Caso | N | Opt. BnB | Opt. ILP | Variables ILP |
|------|---|----------|----------|---------------|
| 1    | 6 | 42       | 42       | 20            |
| 2    | 9 | 73       | 73       | 84            |

**Casos de experimentacion** (`pruebas/experimentos.txt`):

| Caso | N  | Opt. BnB | Opt. ILP | T. BnB (ms) | T. ILP (ms) | Var. ILP |
|------|----|----------|----------|-------------|-------------|----------|
| 1    | 12 | 67       | 67       | 1           | 22          | 220      |
| 2    | 12 | 66       | 66       | 0           | 26          | 220      |
| 3    | 12 | 62       | 62       | 1           | 22          | 220      |
| 4    | 15 | 83       | 83       | 8           | 27          | 455      |
| 5    | 15 | 68       | 68       | 12          | 27          | 455      |
| 6    | 15 | 89       | 89       | 19          | 25          | 455      |
| 7    | 18 | 74       | 74       | 88          | 34          | 816      |
| 8    | 18 | 94       | 94       | 264         | 34          | 816      |
| 9    | 18 | 91       | 91       | 384         | 34          | 816      |
| 10   | 18 | 102      | 102      | 823         | 62          | 816      |
| 11   | 18 | 98       | 98       | 528         | 31          | 816      |

### 4.4 Analisis comparativo

- **Correctitud:** ILP y BnB producen siempre el mismo valor optimo, lo que valida la correccion de ambos enfoques.

- **Escalabilidad del ILP:** el numero de variables crece como \(\binom{N}{3} \sim O(N^3)\), pero el tiempo del solver CBC permanece muy bajo (< 65 ms hasta N=18) porque la relajacion lineal de este ILP de cobertura exacta suele tener soluciones fraccionarias muy cercanas a enteras, lo que hace que el branch-and-cut interno converja rapido.

- **Comparacion de tiempos:**
  - Para N=12, BnB (0-1 ms) es mas rapido que ILP (22-26 ms), pues la sobrecarga de Python y el arranque del solver CBC domina en instancias pequenas.
  - Para N=15, los tiempos son comparables (BnB 8-19 ms, ILP ~25-27 ms).
  - Para N=18, ILP (31-62 ms) supera claramente a BnB (88-823 ms): el solver de PL aprovecha la estructura del problema de cobertura exacta mejor que el BnB ad hoc para estas instancias.

- **Variabilidad:** BnB muestra alta variabilidad entre instancias del mismo N (88 ms vs 823 ms para N=18) dependiendo de la estructura de la matriz de conflictos, mientras que ILP es mas predecible.

---

## 5. Conclusiones

Se ha implementado una solucion exacta de ramificacion y poda para formar equipos de 3 minimizando conflicto total dirigido.  
El diseno combina:

- estrategia de minimo coste estimado,
- cota inferior admisible,
- cota superior dinamica con completado voraz.

El resultado es correcto (validado en instancias pequenas contra fuerza bruta) y escalable mejor que una busqueda exhaustiva pura gracias al efecto conjunto de ordenacion y poda.
