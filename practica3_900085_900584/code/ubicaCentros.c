/*
 * =============================================================================
 * PRACTICA 3: BUSQUEDA CON RETROCESO - UBICACION DE CENTROS DE URGENCIAS
 * =============================================================================
 *
 * Algoritmia Basica - Grado en Ingenieria Informatica - Curso 2025/26
 * Universidad de Zaragoza
 *
 * Autores: NIA 900085, NIA 900584
 *
 * -----------------------------------------------------------------------------
 * DESCRIPCION
 * -----------------------------------------------------------------------------
 * Dado un grafo no dirigido ponderado que representa localidades conectadas por
 * carreteras, un conjunto de centros de urgencia ya existentes y un numero k de
 * nuevos centros a instalar, este programa encuentra la ubicacion optima de los
 * k nuevos centros que minimiza el peor tiempo de acceso desde cualquier
 * localidad al centro de urgencias mas cercano.
 *
 * Algoritmo:
 *   1) Floyd-Warshall para calcular distancias minimas entre todos los pares.
 *   2) Backtracking combinatorio para probar subconjuntos de k localidades,
 *      con poda por cota superior (si la solucion parcial ya supera la mejor
 *      conocida, se descarta la rama).
 *
 * -----------------------------------------------------------------------------
 * USO
 * -----------------------------------------------------------------------------
 *   ubicaCentros <entrada> <salida>
 *
 *   <entrada>: fichero con los casos de prueba
 *   <salida> : fichero donde se escriben los resultados
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define INF 1000000000
#define MAX_N 500

/* =========================================================================
 * SECCION 1: MEDICION DE TIEMPO
 * ========================================================================= */

static double get_time_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart * 1000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
#endif
}

/* =========================================================================
 * SECCION 2: VARIABLES GLOBALES DEL PROBLEMA
 * ========================================================================= */

static int dist[MAX_N][MAX_N];  /* Matriz de distancias minimas (Floyd-Warshall) */
static int n;                    /* Numero de localidades (vertices) */
static int m;                    /* Numero de carreteras (aristas) */
static int c;                    /* Numero de centros existentes */
static int k;                    /* Numero de nuevos centros a instalar */

static int existentes[MAX_N];    /* 1 si la localidad i ya tiene centro */
static int candidatos[MAX_N];    /* Localidades donde se puede poner un nuevo centro */
static int num_candidatos;

/* Distancia minima de cada localidad al centro existente mas cercano */
static int dist_a_existente[MAX_N];

/* Estado del backtracking */
static int mejor_valor;              /* Mejor (minimo) peor tiempo encontrado */
static int mejor_solucion[MAX_N];    /* Localidades de la mejor solucion */
static int solucion_actual[MAX_N];   /* Solucion parcial en construccion */
static long long nodos_generados;

/* =========================================================================
 * SECCION 3: FLOYD-WARSHALL
 * ========================================================================= */

static void floyd_warshall(void) {
    int i, j, kk;
    for (kk = 0; kk < n; kk++)
        for (i = 0; i < n; i++) {
            if (dist[i][kk] == INF) continue;
            for (j = 0; j < n; j++) {
                if (dist[kk][j] == INF) continue;
                if (dist[i][kk] + dist[kk][j] < dist[i][j])
                    dist[i][j] = dist[i][kk] + dist[kk][j];
            }
        }
}

/* =========================================================================
 * SECCION 4: EVALUACION DE UNA SOLUCION
 * ========================================================================= */

/*
 * Calcula el peor tiempo de acceso considerando los centros existentes
 * y los nuevos centros dados en sol[0..num_nuevos-1].
 * Para cada localidad, su tiempo de acceso es el minimo entre:
 *   - dist_a_existente[v] (al centro existente mas cercano)
 *   - min(dist[v][sol[j]]) para cada nuevo centro j
 */
static int evaluar(const int *sol, int num_nuevos) {
    int peor = 0;
    int v, j;
    for (v = 0; v < n; v++) {
        int mejor = dist_a_existente[v];
        for (j = 0; j < num_nuevos; j++) {
            if (dist[v][sol[j]] < mejor)
                mejor = dist[v][sol[j]];
        }
        if (mejor > peor)
            peor = mejor;
    }
    return peor;
}

/*
 * Cota inferior parcial: con los centros ya elegidos (sol[0..nivel-1]),
 * calcula el peor tiempo actual. Cualquier extension solo puede mejorar
 * (reducir) el tiempo de algunas localidades, pero nunca empeorar las
 * que ya tienen buen acceso. Si el peor tiempo ACTUAL ya es >= mejor_valor,
 * no merece la pena seguir (la poda por cota no puede mejorar).
 *
 * Poda adicional: para las localidades cuya distancia minima al conjunto
 * actual (existentes + parcial) ya supera mejor_valor, contamos cuantas son.
 * Si esas localidades no pueden ser cubiertas por los k-nivel centros
 * restantes, podamos. (Simplificado: usamos solo la cota directa.)
 */
static int cota_parcial(const int *sol, int nivel) {
    int peor = 0;
    int v, j;
    for (v = 0; v < n; v++) {
        int mejor = dist_a_existente[v];
        for (j = 0; j < nivel; j++) {
            if (dist[v][sol[j]] < mejor)
                mejor = dist[v][sol[j]];
        }
        if (mejor > peor)
            peor = mejor;
    }
    return peor;
}

/* =========================================================================
 * SECCION 5: BACKTRACKING
 * =========================================================================
 *
 * Representacion de la solucion:
 *   Tupla (s1, s2, ..., sk) donde si es el indice en el array candidatos[].
 *   Restriccion: s1 < s2 < ... < sk (combinaciones, sin repeticion).
 *
 * Arbol de busqueda:
 *   Nivel 0..k-1. En el nivel 'nivel', se elige el candidato con indice
 *   desde 'inicio' hasta num_candidatos - (k - nivel) (para que queden
 *   suficientes candidatos para los niveles restantes).
 *   Tamano del arbol: C(num_candidatos, k) hojas.
 *
 * Poda:
 *   - Si la cota parcial (peor tiempo con los centros ya elegidos) ya es
 *     >= mejor_valor, no merece la pena explorar mas (los centros restantes
 *     solo pueden mejorar algunas localidades, pero si la actual ya no
 *     mejora, descartamos). NOTA: realmente la parcial SI puede mejorar al
 *     anadir mas centros. Usamos una poda mas conservadora pero util:
 *     calculamos la cota con los centros actuales. Si ya iguala o supera
 *     la mejor, y queda algun centro por colocar, aun podria mejorar.
 *     Pero si el peor nodo actual es uno que NINGUN candidato restante puede
 *     mejorar, podamos.
 *   - Poda simple y efectiva: evaluamos la solucion parcial. Si su valor
 *     ya >= mejor_valor, como aun faltan centros podria mejorar, asi que
 *     NO podamos directamente. En cambio, al completar (nivel == k),
 *     actualizamos mejor_valor si mejora.
 *   - Poda greedy: si la cota parcial ya es <= mejor_valor (ya es buena),
 *     no necesariamente podamos, pero si al nivel actual el peor tiempo
 *     no puede reducirse por debajo de mejor_valor con los candidatos
 *     restantes, podamos.
 *
 * Implementacion practica: usamos una poda simple pero efectiva:
 *   En cada nodo, calculamos el peor tiempo con la solucion parcial.
 *   Si este ya es >= mejor_valor, seguimos (aun podria mejorar al anadir
 *   mas centros). Pero si nivel == k, actualizamos. La poda real se hace
 *   comparando: si evaluar la solucion parcial da un valor >= mejor_valor
 *   Y no quedan centros por poner, podamos. Ademas, si hay una sola
 *   localidad cuya distancia minima a TODOS los candidatos restantes
 *   es >= mejor_valor, podamos.
 *
 * Para simplificar y ser eficientes, usamos:
 *   1) Si nivel == k: evaluar completa y actualizar.
 *   2) Si nivel < k: calcular cota_parcial. Si esta cota < mejor_valor,
 *      la solucion parcial ya es buena, seguir. Si >= mejor_valor,
 *      como faltan centros, podria mejorar pero usamos heuristica de poda.
 */

static void backtracking(int nivel, int inicio) {
    nodos_generados++;

    if (nivel == k) {
        int val = evaluar(solucion_actual, k);
        if (val < mejor_valor) {
            mejor_valor = val;
            memcpy(mejor_solucion, solucion_actual, k * sizeof(int));
        }
        return;
    }

    int remaining = k - nivel;
    int limit = num_candidatos - remaining;
    int i;

    for (i = inicio; i <= limit; i++) {
        solucion_actual[nivel] = candidatos[i];

        /*
         * Poda: con los centros elegidos hasta ahora (nivel+1 centros),
         * si el peor tiempo ya es >= mejor_valor, aun podriamos mejorar
         * anadiendo mas centros. Pero podemos hacer una poda parcial:
         * calculamos el peor tiempo de la solucion parcial actual.
         * Si con nivel+1 centros el peor tiempo ya es >= mejor_valor,
         * y los candidatos restantes no mejoran esa localidad critica,
         * podamos. Simplificacion: si cota_parcial >= mejor_valor,
         * verificamos si hay alguna localidad cuyo tiempo minimo al
         * conjunto (existentes + parcial) es >= mejor_valor y ningun
         * candidato restante puede acercarla. Si no, seguimos.
         *
         * Poda efectiva simplificada: evaluamos parcialmente.
         * Si la cota ya es >= mejor_valor, aun NO podamos (quedan centros).
         * Solo podamos si se dan condiciones irrecuperables.
         */
        int cota = cota_parcial(solucion_actual, nivel + 1);

        /* Si la cota parcial ya mejora (es menor que) la mejor conocida,
         * no hace falta podar, continuar explorando */
        if (cota < mejor_valor) {
            backtracking(nivel + 1, i + 1);
        } else {
            /*
             * La cota parcial >= mejor_valor. Aun quedan centros.
             * Intentamos ver si los candidatos restantes pueden mejorar
             * la localidad critica. Si no, podamos.
             */
            if (remaining == 1) {
                /* No quedan mas centros tras este: la cota parcial completa
                 * es la evaluacion final, y ya es >= mejor_valor -> podar */
                continue;
            }

            /* Con mas centros por poner, la peor localidad podria mejorar.
             * Poda parcial: encontrar la localidad critica (la del peor tiempo)
             * y comprobar si algun candidato futuro puede reducir su distancia
             * por debajo de mejor_valor. */
            int v, j;
            int localidad_critica = -1;
            int peor_tiempo = 0;
            for (v = 0; v < n; v++) {
                int mejor = dist_a_existente[v];
                for (j = 0; j <= nivel; j++) {
                    if (dist[v][solucion_actual[j]] < mejor)
                        mejor = dist[v][solucion_actual[j]];
                }
                if (mejor > peor_tiempo) {
                    peor_tiempo = mejor;
                    localidad_critica = v;
                }
            }

            if (localidad_critica >= 0 && peor_tiempo >= mejor_valor) {
                int puede_mejorar = 0;
                for (j = i + 1; j < num_candidatos; j++) {
                    if (dist[localidad_critica][candidatos[j]] < mejor_valor) {
                        puede_mejorar = 1;
                        break;
                    }
                }
                if (!puede_mejorar)
                    continue;
            }

            backtracking(nivel + 1, i + 1);
        }
    }
}

/* =========================================================================
 * SECCION 6: SOLUCION GREEDY INICIAL (para tener una buena cota de partida)
 * =========================================================================
 *
 * Heuristica voraz: en cada paso, colocar el nuevo centro en la localidad
 * que mas reduce el peor tiempo de acceso. Esto da una solucion razonable
 * para inicializar mejor_valor y mejorar la poda del backtracking.
 */

static void solucion_greedy_inicial(void) {
    int usados[MAX_N];
    int nuevos[MAX_N];
    int paso, i, j;

    memset(usados, 0, sizeof(usados));
    for (i = 0; i < n; i++)
        if (existentes[i]) usados[i] = 1;

    int dist_actual[MAX_N];
    for (i = 0; i < n; i++)
        dist_actual[i] = dist_a_existente[i];

    for (paso = 0; paso < k; paso++) {
        int mejor_cand = -1;
        int mejor_peor = INF;

        for (j = 0; j < num_candidatos; j++) {
            int cand = candidatos[j];
            if (usados[cand]) continue;

            int peor = 0;
            for (i = 0; i < n; i++) {
                int d = dist_actual[i];
                if (dist[i][cand] < d) d = dist[i][cand];
                if (d > peor) peor = d;
            }
            if (peor < mejor_peor) {
                mejor_peor = peor;
                mejor_cand = cand;
            }
        }

        if (mejor_cand >= 0) {
            nuevos[paso] = mejor_cand;
            usados[mejor_cand] = 1;
            for (i = 0; i < n; i++) {
                if (dist[i][mejor_cand] < dist_actual[i])
                    dist_actual[i] = dist[i][mejor_cand];
            }
        }
    }

    int val = 0;
    for (i = 0; i < n; i++)
        if (dist_actual[i] > val) val = dist_actual[i];

    if (val < mejor_valor) {
        mejor_valor = val;
        memcpy(mejor_solucion, nuevos, k * sizeof(int));
    }
}

/* =========================================================================
 * SECCION 7: RESOLVER UN CASO DE PRUEBA
 * ========================================================================= */

static void resolver_caso(FILE *fin, FILE *fout) {
    int i, j, v, w, t;

    fscanf(fin, "%d %d %d %d", &n, &m, &c, &k);

    /* Inicializar matriz de distancias */
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            dist[i][j] = (i == j) ? 0 : INF;

    /* Leer aristas (vertices 1-indexed -> convertir a 0-indexed) */
    for (i = 0; i < m; i++) {
        fscanf(fin, "%d %d %d", &v, &w, &t);
        v--; w--;
        if (t < dist[v][w]) {
            dist[v][w] = t;
            dist[w][v] = t;
        }
    }

    /* Leer centros existentes */
    memset(existentes, 0, sizeof(existentes));
    for (i = 0; i < c; i++) {
        fscanf(fin, "%d", &v);
        v--;
        existentes[v] = 1;
    }

    /* Floyd-Warshall */
    floyd_warshall();

    /* Calcular distancia de cada localidad al centro existente mas cercano */
    for (i = 0; i < n; i++) {
        dist_a_existente[i] = INF;
        for (j = 0; j < n; j++) {
            if (existentes[j] && dist[i][j] < dist_a_existente[i])
                dist_a_existente[i] = dist[i][j];
        }
    }

    /* Construir lista de candidatos (localidades sin centro existente) */
    num_candidatos = 0;
    for (i = 0; i < n; i++) {
        if (!existentes[i])
            candidatos[num_candidatos++] = i;
    }

    /* Inicializar backtracking */
    mejor_valor = INF;
    nodos_generados = 0;
    memset(mejor_solucion, 0, sizeof(mejor_solucion));
    memset(solucion_actual, 0, sizeof(solucion_actual));

    double t_inicio = get_time_ms();

    /* Solucion greedy para una buena cota inicial */
    if (k > 0 && num_candidatos >= k)
        solucion_greedy_inicial();

    /* Backtracking con poda */
    if (k > 0 && num_candidatos >= k)
        backtracking(0, 0);

    double t_fin = get_time_ms();
    double tiempo_ms = t_fin - t_inicio;

    /* Escribir resultado: tiempo_ms n_nodos valor_optimo s1 s2 ... sk */
    fprintf(fout, "%.2f %lld %d", tiempo_ms, nodos_generados, mejor_valor);

    /* Ordenar solucion (ya deberia estar ordenada por construccion,
     * pero la greedy puede no estarlo) */
    int sol_ord[MAX_N];
    memcpy(sol_ord, mejor_solucion, k * sizeof(int));
    for (i = 0; i < k - 1; i++)
        for (j = i + 1; j < k; j++)
            if (sol_ord[i] > sol_ord[j]) {
                int tmp = sol_ord[i];
                sol_ord[i] = sol_ord[j];
                sol_ord[j] = tmp;
            }

    for (i = 0; i < k; i++)
        fprintf(fout, " %d", sol_ord[i] + 1);  /* 1-indexed */

    fprintf(fout, "\n");
}

/* =========================================================================
 * SECCION 8: MAIN
 * ========================================================================= */

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <entrada> <salida>\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        fprintf(stderr, "Error: no se pudo abrir '%s'\n", argv[1]);
        return 1;
    }

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        fprintf(stderr, "Error: no se pudo abrir '%s' para escritura\n", argv[2]);
        fclose(fin);
        return 1;
    }

    int num_casos;
    fscanf(fin, "%d", &num_casos);

    int caso;
    for (caso = 0; caso < num_casos; caso++)
        resolver_caso(fin, fout);

    fclose(fin);
    fclose(fout);
    return 0;
}
