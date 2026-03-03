/*
 * =============================================================================
 * PRACTICA 2: PROGRAMACION DINAMICA - PARTICION DE PALABRAS
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
 * Dada una cadena de entrada y un diccionario de palabras, este programa
 * determina si la cadena puede dividirse en una secuencia de palabras del
 * diccionario separadas por espacios. En caso afirmativo, lista TODAS las
 * particiones posibles.
 *
 * El algoritmo se basa en la tecnica de programacion dinamica, considerando
 * todos los prefijos del texto y buscandolos en el diccionario. Si un prefijo
 * esta en el diccionario, se busca recursivamente la particion del resto.
 *
 * -----------------------------------------------------------------------------
 * USO
 * -----------------------------------------------------------------------------
 *   separarPalabras <var> <diccionario> <texto>
 *
 *   <var>        : Variante del algoritmo
 *                   1 = Solucion recursiva pura
 *                   2 = Recursiva con memoizacion
 *                   3 = Iterativa con tabla (bottom-up)
 *   <diccionario>: Ruta al fichero con palabras (una por linea)
 *   <texto>      : Ruta al fichero con la cadena a verificar (sin espacios)
 *
 * -----------------------------------------------------------------------------
 * EJEMPLO
 * -----------------------------------------------------------------------------
 *   Diccionario: {me, gusta, sol, dar, soldar, ...}
 *   Entrada "megustasoldar" -> Si. Particiones: "me gusta soldar", "me gusta sol dar"
 *   Entrada "megustadolar"  -> No.
 *
 * =============================================================================
 */

#define _POSIX_C_SOURCE 200809L  /* Para strdup() en sistemas POSIX (ej: lab000) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Constantes de tamaño maxima para evitar desbordamientos */
#define MAX_PALABRA 256        /* Longitud maxima de una palabra en el diccionario */
#define MAX_TEXTO 10000        /* Longitud maxima de la cadena de entrada */
#define HASH_SIZE 4096         /* Numero de cubos en la tabla hash (potencia de 2) */
#define MAX_PARTICIONES 10000  /* Maximo numero de particiones distintas a mostrar */
#define MAX_PALABRAS_PARTICION 500  /* Maximo palabras en una particion (no usado activamente) */

/*
 * =============================================================================
 * SECCION 1: DICCIONARIO (TABLA HASH)
 * =============================================================================
 *
 * Se utiliza una tabla hash para almacenar el diccionario de forma eficiente.
 * La busqueda de una palabra en el diccionario debe ser O(1) promedio para
 * que el algoritmo sea eficiente (en lugar de O(m) con busqueda lineal).
 *
 * Funcion hash: djb2, ampliamente usada por su buena distribucion con cadenas.
 * Resolucion de colisiones: encadenamiento (lista enlazada por cubo).
 * =============================================================================
 */

/* Nodo de la lista enlazada para resolver colisiones en la tabla hash */
typedef struct NodoHash {
    char *palabra;
    struct NodoHash *sig;
} NodoHash;

/* Estructura del diccionario: array de listas (tabla hash con encadenamiento) */
typedef struct {
    NodoHash *buckets[HASH_SIZE];
    int num_palabras;
} Diccionario;

/*
 * hash: Calcula el indice del cubo para una cadena.
 * Algoritmo djb2: h = 5381; para cada caracter: h = ((h << 5) + h) + c;
 */
static unsigned long hash(const char *str) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        h = ((h << 5) + h) + c;
    return h % HASH_SIZE;
}

/* Inicializa el diccionario vacio (todas las listas a NULL) */
void dicc_iniciar(Diccionario *d) {
    int i;
    for (i = 0; i < HASH_SIZE; i++)
        d->buckets[i] = NULL;
    d->num_palabras = 0;
}

/* Devuelve 1 si la palabra esta en el diccionario, 0 si no */
int dicc_contiene(const Diccionario *d, const char *palabra) {
    unsigned long idx = hash(palabra);
    NodoHash *n = d->buckets[idx];
    while (n) {
        if (strcmp(n->palabra, palabra) == 0)
            return 1;
        n = n->sig;
    }
    return 0;
}

/* Inserta una nueva palabra en el diccionario (al inicio de la lista del cubo) */
void dicc_insertar(Diccionario *d, const char *palabra) {
    unsigned long idx = hash(palabra);
    NodoHash *n = malloc(sizeof(NodoHash));
    if (!n) { perror("malloc"); exit(1); }
    n->palabra = strdup(palabra);
    n->sig = d->buckets[idx];
    d->buckets[idx] = n;
    d->num_palabras++;
}

/* Libera toda la memoria utilizada por el diccionario */
void dicc_liberar(Diccionario *d) {
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        NodoHash *n = d->buckets[i];
        while (n) {
            NodoHash *sig = n->sig;
            free(n->palabra);
            free(n);
            n = sig;
        }
    }
}

/*
 * =============================================================================
 * SECCION 2: ESTRUCTURAS PARA ALMACENAR PARTICIONES
 * =============================================================================
 *
 * Una particion es una forma de segmentar la cadena, ej: "me gusta soldar".
 * El resultado puede tener varias particiones posibles (ej: "me gusta sol dar").
 * ListaParticiones guarda todas las cadenas con las particiones encontradas.
 * =============================================================================
 */

typedef struct {
    char *lineas[MAX_PARTICIONES];  /* Array de cadenas, cada una es una particion */
    int n;                           /* Numero de particiones almacenadas */
} ListaParticiones;

/* Inicializa la lista vacia */
void lista_particiones_init(ListaParticiones *lp) {
    lp->n = 0;
}

/* Aniade una nueva particion a la lista (hace copia con strdup) */
void lista_particiones_append(ListaParticiones *lp, const char *s) {
    if (lp->n >= MAX_PARTICIONES) return;
    lp->lineas[lp->n] = strdup(s);
    lp->n++;
}

/* Libera la memoria de todas las particiones almacenadas */
void lista_particiones_liberar(ListaParticiones *lp) {
    int i;
    for (i = 0; i < lp->n; i++)
        free(lp->lineas[i]);
}

/*
 * =============================================================================
 * SECCION 3: VARIANTE 1 - SOLUCION RECURSIVA PURA
 * =============================================================================
 *
 * Implementacion directa de la ecuacion de recurrencia. No almacena resultados
 * intermedios, por lo que puede recalcular el mismo subproblema muchas veces.
 * Complejidad: exponencial en el peor caso (muchos prefijos validos).
 *
 * ECUACION DE RECURRENCIA:
 *   puede(i) = true   si i >= n  (caso base: sufijo vacio, siempre particionable)
 *   puede(i) = OR para j desde i hasta n-1:
 *                (texto[i..j] esta en diccionario) AND puede(j+1)
 *
 * Es decir: el sufijo texto[i..n-1] se puede particionar si existe algun j
 * tal que el prefijo texto[i..j] es una palabra del diccionario y el resto
 * texto[j+1..n-1] tambien se puede particionar.
 * =============================================================================
 */

/*
 * puede_recursivo: Determina si el sufijo texto[i..n-1] puede particionarse.
 * Devuelve 1 si es posible, 0 si no.
 */
static int puede_recursivo(const char *texto, int n, int i,
                           const Diccionario *dicc) {
    /* Caso base: cadena vacia siempre se puede "particionar" (ya terminamos) */
    if (i >= n) return 1;

    int j;
    char buf[MAX_PALABRA];

    /* Probar todos los prefijos posibles texto[i..j] */
    for (j = i; j < n; j++) {
        int len = j - i + 1;
        if (len >= MAX_PALABRA) continue;

        /* Extraer el prefijo texto[i..j] a un buffer */
        strncpy(buf, texto + i, len);
        buf[len] = '\0';

        /* Si este prefijo es palabra del diccionario Y el resto se puede particionar */
        if (dicc_contiene(dicc, buf) && puede_recursivo(texto, n, j + 1, dicc))
            return 1;
    }
    return 0;  /* Ningun prefijo valido encontrado */
}

/*
 * particiones_recursivo: Construye TODAS las particiones posibles mediante
 * backtracking. Para cada prefijo valido, lo añaade a 'actual' y explora
 * recursivamente el resto. Cuando se llega al final (i >= n), guarda la
 * particion completa en resultado.
 *
 * actual: buffer donde se construye la particion actual (ej: "me gusta ")
 * pos_actual: posicion donde continuar escribiendo en actual
 */
static void particiones_recursivo(const char *texto, int n, int i,
                                  const Diccionario *dicc, char *actual,
                                  int pos_actual, ListaParticiones *resultado) {
    if (i >= n) {
        /* Caso base: hemos terminado de construir una particion */
        actual[pos_actual] = '\0';
        lista_particiones_append(resultado, actual);
        return;
    }

    char buf[MAX_PALABRA];
    int j;

    /* Probar cada prefijo texto[i..j] que este en el diccionario */
    for (j = i; j < n; j++) {
        int len = j - i + 1;
        if (len >= MAX_PALABRA) continue;

        strncpy(buf, texto + i, len);
        buf[len] = '\0';

        if (dicc_contiene(dicc, buf)) {
            int nueva_pos = pos_actual;

            /* Separador entre palabras (espacio), salvo antes de la primera */
            if (nueva_pos > 0)
                actual[nueva_pos++] = ' ';

            /* Copiar la palabra al buffer y continuar con el sufijo */
            strcpy(actual + nueva_pos, buf);
            nueva_pos += len;

            particiones_recursivo(texto, n, j + 1, dicc, actual, nueva_pos, resultado);
        }
    }
}

/*
 * =============================================================================
 * SECCION 4: VARIANTE 2 - RECURSIVA CON MEMOIZACION
 * =============================================================================
 *
 * Memoizacion (o "memoria"): almacenar los resultados de puede(i) en una tabla
 * para evitar recalcular el mismo subproblema. Antes de calcular, se consulta
 * la tabla; si ya esta calculado, se devuelve directamente.
 *
 * Esto reduce la complejidad de exponencial a O(n^2) en tiempo, ya que hay
 * como maximo n subproblemas distintos (uno por cada indice i) y cada uno
 * se calcula una sola vez. Memoria adicional: O(n).
 *
 * La construccion de particiones sigue siendo recursiva (igual que variante 1);
 * la memoizacion solo acelera la decision "¿se puede particionar?".
 * =============================================================================
 */

#define MEM_UNSET -1  /* Valor que indica que aun no se ha calculado puede(i) */

/*
 * puede_memo: Igual que puede_recursivo pero consultando/guardando en mem[i].
 * mem[i] = 1 si puede, 0 si no, MEM_UNSET si aun no calculado.
 */
static int puede_memo(const char *texto, int n, int i, const Diccionario *dicc,
                      int *mem) {
    if (i >= n) return 1;

    /* Si ya lo calculamos antes, devolver el valor almacenado */
    if (mem[i] != MEM_UNSET) return mem[i];

    int j;
    char buf[MAX_PALABRA];

    for (j = i; j < n; j++) {
        int len = j - i + 1;
        if (len >= MAX_PALABRA) continue;
        strncpy(buf, texto + i, len);
        buf[len] = '\0';

        if (dicc_contiene(dicc, buf) && puede_memo(texto, n, j + 1, dicc, mem)) {
            mem[i] = 1;  /* Guardar resultado para futuras consultas */
            return 1;
        }
    }

    mem[i] = 0;  /* No es posible particionar desde i */
    return 0;
}

/* Construye las particiones usando la misma logica recursiva que la variante 1 */
static void particiones_con_memo(const char *texto, int n, const Diccionario *dicc,
                                 ListaParticiones *resultado) {
    char actual[MAX_TEXTO];
    particiones_recursivo(texto, n, 0, dicc, actual, 0, resultado);
}

/*
 * =============================================================================
 * SECCION 5: VARIANTE 3 - SOLUCION ITERATIVA CON TABLA (BOTTOM-UP)
 * =============================================================================
 *
 * En lugar de recursion, se rellena una tabla de forma ordenada. El calculo
 * va desde los sufijos mas cortos (indice n) hacia los mas largos (indice 0).
 * Asi, cuando calculamos posible[i], ya tenemos calculado posible[j+1] para
 * todo j >= i.
 *
 * Tabla: posible[i] = 1 si el sufijo texto[i..n-1] se puede particionar
 *        posible[n] = 1 (caso base: cadena vacia)
 *
 * Orden de calculo: i = n-1, n-2, ..., 1, 0 (de derecha a izquierda)
 *
 * Coste: O(n^2) en tiempo, O(n) en espacio para la tabla.
 * Es la version mas eficiente ya que evita la sobrecarga de llamadas recursivas.
 * =============================================================================
 */

/*
 * llenar_tabla: Rellena el array posible[0..n] con los valores de la recurrencia.
 * posible[n] = 1 (base). Para i < n: posible[i] = 1 si existe j con palabra
 * valida y posible[j+1] = 1.
 */
static void llenar_tabla(const char *texto, int n, const Diccionario *dicc,
                         int *posible) {
    int i, j;
    char buf[MAX_PALABRA];
    posible[n] = 1;  /* Caso base: sufijo vacio siempre particionable */

    /* Rellenar de atras hacia adelante (i = n-1 hasta 0) */
    for (i = n - 1; i >= 0; i--) {
        posible[i] = 0;

        for (j = i; j < n; j++) {
            int len = j - i + 1;
            if (len >= MAX_PALABRA) continue;

            strncpy(buf, texto + i, len);
            buf[len] = '\0';

            /* Si texto[i..j] es palabra Y texto[j+1..n-1] se puede particionar */
            if (dicc_contiene(dicc, buf) && posible[j + 1]) {
                posible[i] = 1;
                break;  /* Basta con una opcion valida */
            }
        }
    }
}

/*
 * particiones_tabla_aux: Construye las particiones usando la tabla 'posible'
 * como guia. Solo explora ramas donde posible[i]=1 (evita ramas muertas).
 * La construccion es recursiva/backtracking igual que en variantes 1 y 2.
 */
static void particiones_tabla_aux(const char *texto, int n, int i,
                                 const Diccionario *dicc, const int *posible,
                                 char *actual, int pos_actual,
                                 ListaParticiones *resultado) {
    if (i >= n) {
        actual[pos_actual] = '\0';
        lista_particiones_append(resultado, actual);
        return;
    }

    /* Optimizacion: si la tabla dice que no se puede, no explorar */
    if (!posible[i]) return;
    char buf[MAX_PALABRA];
    int j;
    for (j = i; j < n; j++) {
        int len = j - i + 1;
        if (len >= MAX_PALABRA) continue;
        strncpy(buf, texto + i, len);
        buf[len] = '\0';
        if (dicc_contiene(dicc, buf) && posible[j + 1]) {
            int nueva_pos = pos_actual;
            if (nueva_pos > 0) actual[nueva_pos++] = ' ';
            strcpy(actual + nueva_pos, buf);
            nueva_pos += len;
            particiones_tabla_aux(texto, n, j + 1, dicc, posible, actual,
                                  nueva_pos, resultado);
        }
    }
}

/*
 * =============================================================================
 * SECCION 6: CARGA DE FICHEROS
 * =============================================================================
 *
 * cargar_diccionario: Lee un fichero con una palabra por linea (o separadas
 *                    por espacios) e inserta cada palabra en la tabla hash.
 *
 * cargar_texto: Lee la cadena a verificar. Elimina espacios, saltos de linea
 *               y tabulaciones para obtener la cadena sin espacios.
 * =============================================================================
 */

/* Carga el diccionario desde fichero. Devuelve 0 si OK, -1 si error */
static int cargar_diccionario(const char *fichero, Diccionario *dicc) {
    FILE *f = fopen(fichero, "r");
    if (!f) {
        fprintf(stderr, "Error: no se pudo abrir el diccionario '%s'\n", fichero);
        return -1;
    }
    dicc_iniciar(dicc);
    char buf[MAX_PALABRA];

    /* Leer palabra a palabra (fscanf salta espacios automaticamente) */
    while (fscanf(f, "%255s", buf) == 1) {
        /* Eliminar caracteres no alfabeticos al final (puntuacion, etc.) */
        int len = strlen(buf);
        while (len > 0 && !isalpha((unsigned char)buf[len-1])) {
            buf[--len] = '\0';
        }
        if (len > 0)
            dicc_insertar(dicc, buf);
    }
    fclose(f);
    return 0;
}

/* Carga el texto desde fichero (eliminando espacios). Devuelve longitud o -1 si error */
static int cargar_texto(const char *fichero, char *texto, int max_len) {
    FILE *f = fopen(fichero, "r");
    if (!f) {
        fprintf(stderr, "Error: no se pudo abrir el texto '%s'\n", fichero);
        return -1;
    }
    int i = 0;
    int c;

    /* Leer caracter a caracter, ignorando espacios y saltos de linea */
    while (i < max_len - 1 && (c = fgetc(f)) != EOF) {
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            continue;  /* Omitir blancos */
        texto[i++] = (char)c;
    }
    texto[i] = '\0';
    fclose(f);
    return i;
}

/*
 * =============================================================================
 * SECCION 7: FUNCION MAIN
 * =============================================================================
 *
 * Flujo: 1) Validar argumentos
 *        2) Cargar diccionario y texto
 *        3) Ejecutar variante seleccionada (1, 2 o 3)
 *        4) Mostrar resultado (Si/No y lista de particiones si aplica)
 *        5) Liberar memoria
 * =============================================================================
 */

int main(int argc, char **argv) {
    /* ---------- 1. Validar argumentos ---------- */
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <var> <diccionario> <texto>\n", argv[0]);
        fprintf(stderr, "  var: 1 (recursivo), 2 (recursivo con memoria), 3 (tabla)\n");
        return 1;
    }

    int var = atoi(argv[1]);
    if (var < 1 || var > 3) {
        fprintf(stderr, "Error: var debe ser 1, 2 o 3\n");
        return 1;
    }

    const char *fichero_dicc = argv[2];
    const char *fichero_texto = argv[3];

    /* ---------- 2. Cargar diccionario y texto ---------- */
    Diccionario dicc;
    if (cargar_diccionario(fichero_dicc, &dicc) < 0)
        return 1;

    char texto[MAX_TEXTO];
    int n = cargar_texto(fichero_texto, texto, MAX_TEXTO);
    if (n < 0) {
        dicc_liberar(&dicc);
        return 1;
    }

    /* ---------- 3. Ejecutar algoritmo segun variante ---------- */
    int puede = 0;
    ListaParticiones particiones;
    lista_particiones_init(&particiones);

    if (var == 1) {
        /* Variante 1: recursivo puro */
        puede = puede_recursivo(texto, n, 0, &dicc);
        if (puede)
            particiones_recursivo(texto, n, 0, &dicc,
                                 (char[MAX_TEXTO]){0}, 0, &particiones);
    } else if (var == 2) {
        /* Variante 2: recursivo con memoizacion */
        int *mem = calloc((size_t)(n + 1), sizeof(int));
        if (!mem) { perror("malloc"); dicc_liberar(&dicc); return 1; }
        {
            int i;
            for (i = 0; i <= n; i++) mem[i] = MEM_UNSET;
        }
        puede = puede_memo(texto, n, 0, &dicc, mem);
        if (puede)
            particiones_con_memo(texto, n, &dicc, &particiones);
        free(mem);
    } else {
        /* Variante 3: iterativo con tabla */
        int *posible = calloc((size_t)(n + 1), sizeof(int));
        if (!posible) { perror("malloc"); dicc_liberar(&dicc); return 1; }
        llenar_tabla(texto, n, &dicc, posible);
        puede = posible[0];
        if (puede) {
            char actual[MAX_TEXTO];
            particiones_tabla_aux(texto, n, 0, &dicc, posible, actual, 0, &particiones);
        }
        free(posible);
    }

    /* ---------- 4. Mostrar resultado ---------- */
    if (puede) {
        printf("Si.\n");
        printf("La cadena se puede segmentar como:\n");
        int i;
        for (i = 0; i < particiones.n; i++)
            printf(" - '%s'\n", particiones.lineas[i]);
    } else {
        printf("No.\n");
    }

    /* ---------- 5. Liberar memoria ---------- */
    lista_particiones_liberar(&particiones);
    dicc_liberar(&dicc);
    return 0;
}
