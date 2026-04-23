#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Estado global del verificador exacto por backtracking. */
typedef struct {
    int **conflicts;
    int n;
    int best;
    long long nodes;
} BFContext;

/* Mismo coste de equipo que en la implementacion principal. */
static inline int team_cost(int **c, int x, int y, int z) {
    return c[x][y] + c[x][z] + c[y][x] + c[y][z] + c[z][x] + c[z][y];
}

/* Primer participante no asignado en la mascara actual. */
static int first_free(int n, uint64_t mask) {
    for (int i = 0; i < n; ++i) {
        if (((mask >> i) & 1ULL) == 0ULL) {
            return i;
        }
    }
    return -1;
}

/*
 * Backtracking exhaustivo:
 * genera todas las particiones en ternas y mantiene el mejor coste global.
 */
static void backtrack(BFContext *ctx, uint64_t used_mask, int cost) {
    /* Poda basica: no continuar si ya no puede mejorar el mejor conocido. */
    if (cost >= ctx->best) {
        return;
    }
    int i = first_free(ctx->n, used_mask);
    if (i == -1) {
        if (cost < ctx->best) {
            ctx->best = cost;
        }
        return;
    }
    for (int j = i + 1; j < ctx->n; ++j) {
        if ((used_mask >> j) & 1ULL) {
            continue;
        }
        for (int k = j + 1; k < ctx->n; ++k) {
            if ((used_mask >> k) & 1ULL) {
                continue;
            }
            ctx->nodes++;
            int c = team_cost(ctx->conflicts, i, j, k);
            backtrack(ctx, used_mask | (1ULL << i) | (1ULL << j) | (1ULL << k), cost + c);
        }
    }
}

/* Reserva matriz N x N. */
static int **alloc_matrix(int n) {
    int **m = (int **)malloc((size_t)n * sizeof(int *));
    for (int i = 0; i < n; ++i) {
        m[i] = (int *)malloc((size_t)n * sizeof(int));
    }
    return m;
}

/* Libera matriz N x N. */
static void free_matrix(int **m, int n) {
    for (int i = 0; i < n; ++i) {
        free(m[i]);
    }
    free(m);
}

/* Lee casos de entrada y escribe los optimos exactos por caso. */
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <entrada> <salida>\n", argv[0]);
        return 1;
    }
    FILE *in = fopen(argv[1], "r");
    if (!in) {
        fprintf(stderr, "No se puede abrir %s\n", argv[1]);
        return 1;
    }
    FILE *out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "No se puede abrir %s\n", argv[2]);
        fclose(in);
        return 1;
    }

    int case_id = 1;
    while (true) {
        int n;
        if (fscanf(in, "%d", &n) != 1) {
            break;
        }
        if (n <= 0 || n % 3 != 0 || n > 18) {
            fprintf(out, "Caso %d\nN=%d no soportado por verificador (1..18 y mod3=0)\n\n", case_id, n);
            for (int skip = 0; skip < n * n; ++skip) {
                int tmp;
                if (fscanf(in, "%d", &tmp) != 1) {
                    break;
                }
            }
            case_id++;
            continue;
        }
        int **c = alloc_matrix(n);
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) {
            for (int j = 0; j < n; ++j) {
                if (fscanf(in, "%d", &c[i][j]) != 1) {
                    ok = false;
                    break;
                }
            }
        }
        if (!ok) {
            free_matrix(c, n);
            break;
        }

        BFContext ctx;
        ctx.conflicts = c;
        ctx.n = n;
        ctx.best = INT_MAX;
        ctx.nodes = 1;

        clock_t t0 = clock();
        backtrack(&ctx, 0ULL, 0);
        clock_t t1 = clock();
        double ms = 1000.0 * (double)(t1 - t0) / (double)CLOCKS_PER_SEC;

        fprintf(out, "Caso %d\n", case_id);
        fprintf(out, "N=%d\n", n);
        fprintf(out, "Tiempo_ms=%.3f\n", ms);
        fprintf(out, "Nodos_explorados=%lld\n", ctx.nodes);
        fprintf(out, "Valor_optimo=%d\n\n", ctx.best);

        free_matrix(c, n);
        case_id++;
    }

    fclose(in);
    fclose(out);
    return 0;
}
