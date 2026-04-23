#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INF_COST 0x3f3f3f3f

/* Equipo de 3 participantes (indices base 0). */
typedef struct {
    int a;
    int b;
    int c;
} Team;

/* Nodo del arbol de busqueda Branch&Bound. */
typedef struct {
    uint64_t used_mask;
    int teams_count;
    int cost_so_far;
    int estimated_cost;
    Team *teams;
} Node;

/* Min-heap de nodos vivos ordenado por coste estimado. */
typedef struct {
    Node **data;
    size_t size;
    size_t capacity;
} MinHeap;

/* Coste dirigido de un equipo: suma de conflictos en ambas direcciones. */
static inline int team_cost(int n, int **conflicts, int x, int y, int z) {
    (void)n;
    return conflicts[x][y] + conflicts[x][z] + conflicts[y][x] + conflicts[y][z] +
           conflicts[z][x] + conflicts[z][y];
}

/* Consulta rapida de participante ya asignado usando mascara de bits. */
static inline bool is_used(uint64_t mask, int idx) { return ((mask >> idx) & 1ULL) != 0ULL; }

/* Devuelve el primer participante aun no asignado en el nodo actual. */
static int first_unassigned(int n, uint64_t mask) {
    for (int i = 0; i < n; ++i) {
        if (!is_used(mask, i)) {
            return i;
        }
    }
    return -1;
}

/*
 * Cota inferior para los participantes pendientes.
 * Para cada participante libre i suma sus dos menores conflictos salientes
 * hacia otros libres. Esta suma subestima el coste adicional real.
 */
static int lower_bound_remaining(int n, int **conflicts, uint64_t used_mask) {
    int lb = 0;
    for (int i = 0; i < n; ++i) {
        if (is_used(used_mask, i)) {
            continue;
        }
        int best1 = INF_COST;
        int best2 = INF_COST;
        for (int j = 0; j < n; ++j) {
            if (i == j || is_used(used_mask, j)) {
                continue;
            }
            int value = conflicts[i][j];
            if (value < best1) {
                best2 = best1;
                best1 = value;
            } else if (value < best2) {
                best2 = value;
            }
        }
        if (best2 == INF_COST) {
            return INF_COST;
        }
        lb += best1 + best2;
    }
    return lb;
}

/*
 * Cota superior heuristica: completa la solucion parcial de forma voraz
 * eligiendo, para el primer libre i, la mejor terna (i,j,k) disponible.
 * No garantiza optimalidad, pero sirve para inicializar/mejorar U.
 */
static int greedy_completion_upper_bound(int n, int **conflicts, uint64_t used_mask, int partial_cost) {
    int total = partial_cost;
    uint64_t mask = used_mask;

    while (true) {
        int i = first_unassigned(n, mask);
        if (i == -1) {
            break;
        }
        int best_cost = INF_COST;
        int best_j = -1;
        int best_k = -1;

        for (int j = i + 1; j < n; ++j) {
            if (is_used(mask, j)) {
                continue;
            }
            for (int k = j + 1; k < n; ++k) {
                if (is_used(mask, k)) {
                    continue;
                }
                int c = team_cost(n, conflicts, i, j, k);
                if (c < best_cost) {
                    best_cost = c;
                    best_j = j;
                    best_k = k;
                }
            }
        }
        if (best_j == -1) {
            return INF_COST;
        }

        total += best_cost;
        mask |= (1ULL << i) | (1ULL << best_j) | (1ULL << best_k);
    }
    return total;
}

/* Inicializa la cola de prioridad de nodos vivos. */
static void heap_init(MinHeap *heap) {
    heap->size = 0;
    heap->capacity = 1024;
    heap->data = (Node **)malloc(heap->capacity * sizeof(Node *));
}

/* Libera memoria interna del heap (no los nodos, esos se liberan aparte). */
static void heap_destroy(MinHeap *heap) {
    free(heap->data);
    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

/* Intercambio auxiliar para operaciones de heap. */
static void heap_swap(Node **a, Node **b) {
    Node *tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Inserta un nodo vivo en el heap manteniendo la propiedad de min-heap. */
static void heap_push(MinHeap *heap, Node *node) {
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->data = (Node **)realloc(heap->data, heap->capacity * sizeof(Node *));
    }
    size_t i = heap->size++;
    heap->data[i] = node;
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (heap->data[parent]->estimated_cost <= heap->data[i]->estimated_cost) {
            break;
        }
        heap_swap(&heap->data[parent], &heap->data[i]);
        i = parent;
    }
}

/* Extrae el nodo vivo con menor coste estimado. */
static Node *heap_pop(MinHeap *heap) {
    if (heap->size == 0) {
        return NULL;
    }
    Node *root = heap->data[0];
    heap->data[0] = heap->data[--heap->size];
    size_t i = 0;
    while (true) {
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        size_t smallest = i;

        if (left < heap->size &&
            heap->data[left]->estimated_cost < heap->data[smallest]->estimated_cost) {
            smallest = left;
        }
        if (right < heap->size &&
            heap->data[right]->estimated_cost < heap->data[smallest]->estimated_cost) {
            smallest = right;
        }
        if (smallest == i) {
            break;
        }
        heap_swap(&heap->data[i], &heap->data[smallest]);
        i = smallest;
    }
    return root;
}

/* Reserva un nodo vacio con espacio para todos los equipos posibles. */
static Node *create_node(int teams_total) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->used_mask = 0ULL;
    node->teams_count = 0;
    node->cost_so_far = 0;
    node->estimated_cost = 0;
    node->teams = (Team *)malloc((size_t)teams_total * sizeof(Team));
    return node;
}

/* Clona un nodo para generar hijos sin modificar el padre. */
static Node *clone_node(const Node *src, int teams_total) {
    Node *node = create_node(teams_total);
    node->used_mask = src->used_mask;
    node->teams_count = src->teams_count;
    node->cost_so_far = src->cost_so_far;
    node->estimated_cost = src->estimated_cost;
    memcpy(node->teams, src->teams, (size_t)teams_total * sizeof(Team));
    return node;
}

/* Libera un nodo y su vector de equipos. */
static void destroy_node(Node *node) {
    if (node != NULL) {
        free(node->teams);
        free(node);
    }
}

/* Reserva una matriz cuadrada N x N de enteros. */
static int **alloc_matrix(int n) {
    int **m = (int **)malloc((size_t)n * sizeof(int *));
    for (int i = 0; i < n; ++i) {
        m[i] = (int *)malloc((size_t)n * sizeof(int));
    }
    return m;
}

/* Libera una matriz cuadrada previamente reservada. */
static void free_matrix(int **m, int n) {
    for (int i = 0; i < n; ++i) {
        free(m[i]);
    }
    free(m);
}

/*
 * Resuelve un caso con Branch&Bound de minimizacion.
 * Tambien mide tiempo y nodos generados para el informe experimental.
 */
static void solve_case(FILE *out, int case_id, int n, int **conflicts) {
    if (n <= 0 || n % 3 != 0 || n > 63) {
        fprintf(out, "Caso %d\n", case_id);
        fprintf(out, "N=%d invalido. Se requiere N > 0, N mod 3 = 0 y N <= 63.\n\n", n);
        return;
    }

    const int teams_total = n / 3;
    clock_t t0 = clock();

    MinHeap heap;
    heap_init(&heap);

    Node *root = create_node(teams_total);
    root->estimated_cost = lower_bound_remaining(n, conflicts, root->used_mask);
    heap_push(&heap, root);

    long long generated_nodes = 1;
    int best_cost = greedy_completion_upper_bound(n, conflicts, 0ULL, 0);
    Team *best_teams = (Team *)malloc((size_t)teams_total * sizeof(Team));
    bool have_best_solution = false;

    while (heap.size > 0) {
        Node *current = heap_pop(&heap);

        /* Poda por cota superior global U (best_cost). */
        if (current->estimated_cost >= best_cost) {
            destroy_node(current);
            continue;
        }

        /* Nodo hoja: si mejora U, se guarda como mejor solucion. */
        if (current->teams_count == teams_total) {
            if (current->cost_so_far < best_cost) {
                best_cost = current->cost_so_far;
                memcpy(best_teams, current->teams, (size_t)teams_total * sizeof(Team));
                have_best_solution = true;
            }
            destroy_node(current);
            continue;
        }

        int i = first_unassigned(n, current->used_mask);
        if (i == -1) {
            destroy_node(current);
            continue;
        }

        for (int j = i + 1; j < n; ++j) {
            if (is_used(current->used_mask, j)) {
                continue;
            }
            for (int k = j + 1; k < n; ++k) {
                if (is_used(current->used_mask, k)) {
                    continue;
                }

                generated_nodes++;
                Node *child = clone_node(current, teams_total);
                child->teams[child->teams_count++] = (Team){i, j, k};
                child->used_mask |= (1ULL << i) | (1ULL << j) | (1ULL << k);
                child->cost_so_far += team_cost(n, conflicts, i, j, k);

                /* Poda por coste acumulado (costes no negativos). */
                if (child->cost_so_far >= best_cost) {
                    destroy_node(child);
                    continue;
                }

                int lb_rem = lower_bound_remaining(n, conflicts, child->used_mask);
                if (lb_rem == INF_COST) {
                    destroy_node(child);
                    continue;
                }
                child->estimated_cost = child->cost_so_far + lb_rem;

                /* Poda clasica de Branch&Bound con cota inferior. */
                if (child->estimated_cost >= best_cost) {
                    destroy_node(child);
                    continue;
                }

                int greedy_ub = greedy_completion_upper_bound(
                    n, conflicts, child->used_mask, child->cost_so_far);
                if (greedy_ub < best_cost) {
                    /* Si la heuristica encuentra una mejor factible, se estrecha U. */
                    best_cost = greedy_ub;
                }

                if (child->teams_count == teams_total) {
                    if (child->cost_so_far < best_cost) {
                        best_cost = child->cost_so_far;
                    }
                    if (!have_best_solution || child->cost_so_far == best_cost) {
                        memcpy(best_teams, child->teams, (size_t)teams_total * sizeof(Team));
                        have_best_solution = true;
                    }
                    destroy_node(child);
                    continue;
                }

                heap_push(&heap, child);
            }
        }

        destroy_node(current);
    }

    if (!have_best_solution) {
        Node *final_check = create_node(teams_total);
        final_check->used_mask = 0ULL;
        final_check->teams_count = 0;
        final_check->cost_so_far = 0;
        final_check->estimated_cost = 0;
        destroy_node(final_check);
    }

    clock_t t1 = clock();
    double elapsed_ms = (1000.0 * (double)(t1 - t0)) / (double)CLOCKS_PER_SEC;

    fprintf(out, "Caso %d\n", case_id);
    fprintf(out, "N=%d\n", n);
    fprintf(out, "Tiempo_ms=%.3f\n", elapsed_ms);
    fprintf(out, "Nodos_generados=%lld\n", generated_nodes);
    fprintf(out, "Valor_optimo=%d\n", best_cost);
    if (have_best_solution) {
        fprintf(out, "Equipos_optimos (indices 1..N):\n");
        for (int t = 0; t < teams_total; ++t) {
            fprintf(out, "  Equipo_%d=(%d,%d,%d)\n", t + 1, best_teams[t].a + 1, best_teams[t].b + 1,
                    best_teams[t].c + 1);
        }
    }
    fprintf(out, "\n");

    while (heap.size > 0) {
        Node *rest = heap_pop(&heap);
        destroy_node(rest);
    }
    heap_destroy(&heap);
    free(best_teams);
}

/* Lee todos los bloques de entrada y resuelve caso a caso. */
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <fichero_entrada> <fichero_salida>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (in == NULL) {
        fprintf(stderr, "Error: no se puede abrir %s\n", argv[1]);
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (out == NULL) {
        fprintf(stderr, "Error: no se puede crear %s\n", argv[2]);
        fclose(in);
        return 1;
    }

    int case_id = 1;
    while (true) {
        int n = 0;
        int r = fscanf(in, "%d", &n);
        if (r != 1) {
            break;
        }
        int **conflicts = alloc_matrix(n);
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) {
            for (int j = 0; j < n; ++j) {
                if (fscanf(in, "%d", &conflicts[i][j]) != 1) {
                    ok = false;
                    break;
                }
            }
        }

        if (!ok) {
            fprintf(out, "Caso %d\nEntrada incompleta.\n\n", case_id);
            free_matrix(conflicts, n);
            break;
        }

        solve_case(out, case_id, n, conflicts);
        free_matrix(conflicts, n);
        case_id++;
    }

    fclose(in);
    fclose(out);
    return 0;
}
