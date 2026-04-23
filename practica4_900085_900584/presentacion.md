# Guia de presentacion - Practica 4 (Ramificacion y poda)

## 1) Problema en 20 segundos

Tenemos `N` participantes y queremos hacer `N/3` equipos de 3 minimizando el conflicto total interno.  
El conflicto es dirigido (`cij` puede ser distinto de `cji`), por eso en cada equipo sumamos las 6 direcciones.

## 2) Decisiones de diseno importantes

### Decision 1: representacion del estado

Usamos:

- mascara de participantes ya asignados,
- lista de equipos ya cerrados,
- coste acumulado.

**Por que:** permite chequeos muy rapidos de "esta persona ya esta asignada?" y simplifica generar hijos.

### Decision 2: forma de ramificar

En cada nodo cogemos el menor indice libre `i` y probamos todas las parejas `(j,k)` libres.

**Por que:** evita duplicados por simetria (no generamos el mismo equipo en distinto orden).

### Decision 3: orden de exploracion

Cola de prioridad por coste estimado \(\hat{c}\) (best-first / minimo coste estimado).

**Por que:** tiende a encontrar antes soluciones buenas, lo que baja `U` y activa podas antes.

### Decision 4: cota inferior admisible

Para cada participante libre, sumamos sus dos conflictos salientes mas pequenos hacia libres.

**Por que:** es barata de calcular y nunca sobreestima, asi que es valida para poda exacta.

### Decision 5: cota superior inicial

Completado voraz desde el estado actual.

**Por que:** tener un `U` pequeno desde el principio reduce mucho nodos explorados.

## 3) Correccion de la solucion

La correccion se apoya en:

- algoritmo exacto de Branch&Bound,
- poda segura (`\hat{c}(x) >= U`),
- comparacion contra un verificador de fuerza bruta en instancias pequenas.

Mensaje para defender: *"No es heuristico para el valor final, solo usamos heuristicas para podar/ordenar. El optimo sigue siendo exacto."*

## 4) Resultados y que destacar

Datos representativos observados en nuestras ejecuciones:

| N  | Tiempo (ms) aprox. | Nodos generados aprox. |
|----|---------------------|------------------------|
| 6  | 0                   | 11                     |
| 9  | 0                   | 259                    |
| 12 | 0-1                 | 1.898-3.786            |
| 15 | 8-19                | 33.257-76.109          |
| 18 | 88-823              | 305.829-3.174.065      |

Guion rapido para comentar esta tabla:

1. En casos pequenos (`N=6,9`), el valor optimo de Branch&Bound coincide con el verificador exacto por fuerza bruta.
2. Al crecer `N`, se dispara el numero de nodos y tambien el tiempo.
3. Aun con ese crecimiento, la poda evita explorar una parte enorme del arbol completo.
4. Entre instancias del mismo `N` hay diferencias claras: la matriz de conflictos condiciona mucho el rendimiento.

## 5) Posibles preguntas del profesor y respuesta corta

- **"Por que no DFS puro?"**  
  Porque no aprovecha tan bien la informacion de coste para encontrar pronto buenas soluciones y podar.

- **"La poda puede eliminar la solucion optima?"**  
  No, porque la cota usada es admisible (subestima), y solo podo cuando ya no puede mejorar `U`.

- **"Que limita el metodo?"**  
  Sigue siendo exponencial en peor caso; para `N` grandes la complejidad crece mucho.

- **"Como validasteis?"**  
  Comparando optimos con fuerza bruta en casos pequenos y revisando consistencia en casos mayores.

## 6) Reparto sugerido de exposicion (si vais dos)

- Persona 1: problema, modelado y diseno Branch&Bound.
- Persona 2: implementacion, pruebas, resultados y conclusiones.
