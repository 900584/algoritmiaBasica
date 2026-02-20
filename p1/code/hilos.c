#define _USE_MATH_DEFINES  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Representa un punto en el plano 2D (coordenadas de clavos)
typedef struct {
    double x, y;
} Punto;

// Representa un hilo entre dos clavos con su beneficio asociado
typedef struct {
    int clavo1, clavo2;  // Indices de los clavos conectados
    double error;        // Beneficio de dibujar este hilo
    double puntuacion;   // Puntuacion para ordenacion
} Hilo;

// Representa una imagen en escala de grises formato PGM
typedef struct {
    int ancho, alto;
    unsigned char **pixels;  // Matriz de pixeles [alto][ancho]
} Imagen;

// Parametros del algoritmo voraz
typedef struct {
    int n;  // Numero de clavos
    int p;  // Numero de hilos candidatos por iteracion
    int s;  // Numero de mejores hilos a seleccionar
} Parametros;

// Lee una imagen en formato PGM desde archivo
Imagen* leer_pgm(const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", nombre_archivo);
        return NULL;
    }
    
    char formato[3];
    if (fscanf(archivo, "%2s", formato) != 1 || strcmp(formato, "P2") != 0) {
        fprintf(stderr, "Error: Formato PGM no valido (debe ser P2)\n");
        fclose(archivo);
        return NULL;
    }
    
    Imagen *img = (Imagen*)malloc(sizeof(Imagen));
    if (img == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        fclose(archivo);
        return NULL;
    }
    
    int max_valor;
    if (fscanf(archivo, "%d %d %d", &img->ancho, &img->alto, &max_valor) != 3) {
        fprintf(stderr, "Error: No se pudo leer dimensiones de la imagen\n");
        free(img);
        fclose(archivo);
        return NULL;
    }
    
    img->pixels = (unsigned char**)malloc(img->alto * sizeof(unsigned char*));
    if (img->pixels == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para pixeles\n");
        free(img);
        fclose(archivo);
        return NULL;
    }
    
    for (int i = 0; i < img->alto; i++) {
        img->pixels[i] = (unsigned char*)malloc(img->ancho * sizeof(unsigned char));
        if (img->pixels[i] == NULL) {
            fprintf(stderr, "Error: No se pudo asignar memoria para fila %d\n", i);
            for (int j = 0; j < i; j++) {
                free(img->pixels[j]);
            }
            free(img->pixels);
            free(img);
            fclose(archivo);
            return NULL;
        }
    }
    
    int valor;
    for (int i = 0; i < img->alto; i++) {
        for (int j = 0; j < img->ancho; j++) {
            if (fscanf(archivo, "%d", &valor) != 1) {
                fprintf(stderr, "Error: No se pudo leer pixel en (%d, %d)\n", j, i);
                for (int k = 0; k < img->alto; k++) {
                    free(img->pixels[k]);
                }
                free(img->pixels);
                free(img);
                fclose(archivo);
                return NULL;
            }
            img->pixels[i][j] = (unsigned char)valor;
        }
    }
    
    fclose(archivo);
    return img;
}

// Escribe una imagen en formato PGM a archivo
int escribir_pgm(const char *nombre_archivo, Imagen *img) {
    FILE *archivo = fopen(nombre_archivo, "w");
    if (archivo == NULL) {
        fprintf(stderr, "Error: No se pudo crear el archivo %s\n", nombre_archivo);
        return 0;
    }
    
    fprintf(archivo, "P2\n");
    fprintf(archivo, "%d %d\n", img->ancho, img->alto);
    fprintf(archivo, "255\n");
    
    for (int i = 0; i < img->alto; i++) {
        for (int j = 0; j < img->ancho; j++) {
            fprintf(archivo, "%d", img->pixels[i][j]);
            if (j < img->ancho - 1) {
                fprintf(archivo, " ");
            }
        }
        fprintf(archivo, "\n");
    }
    
    fclose(archivo);
    return 1;
}

// Libera la memoria asignada para una imagen
void liberar_imagen(Imagen *img) {
    if (img == NULL) return;
    if (img->pixels != NULL) {
        for (int i = 0; i < img->alto; i++) {
            free(img->pixels[i]);
        }
        free(img->pixels);
    }
    free(img);
}

// Genera n clavos distribuidos uniformemente en el perimetro rectangular
Punto* generar_clavos(int n, int ancho, int alto) {
    Punto *clavos = (Punto*)malloc(n * sizeof(Punto));
    if (clavos == NULL) return NULL;
    
    double perimetro = 2.0 * (ancho - 1) + 2.0 * (alto - 1);
    double distancia_entre_clavos = perimetro / n;
    
    for (int i = 0; i < n; i++) {
        double pos = i * distancia_entre_clavos;
        
        if (pos < ancho - 1) {
            clavos[i].x = pos;
            clavos[i].y = 0;
        } else if (pos < (ancho - 1) + (alto - 1)) {
            clavos[i].x = ancho - 1;
            clavos[i].y = pos - (ancho - 1);
        } else if (pos < 2 * (ancho - 1) + (alto - 1)) {
            clavos[i].x = (ancho - 1) - (pos - (ancho - 1) - (alto - 1));
            clavos[i].y = alto - 1;
        } else {
            clavos[i].x = 0;
            clavos[i].y = (alto - 1) - (pos - 2 * (ancho - 1) - (alto - 1));
        }
    }
    
    return clavos;
}

// Dibuja una linea entre dos puntos usando el algoritmo de Bresenham
// Cada pixel se oscurece restando 'valor' (permite acumulacion de hilos)
void dibujar_linea_bresenham(Imagen *img, int x1, int y1, int x2, int y2, 
                             unsigned char valor, int *num_pixels) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1;
    int y = y1;
    *num_pixels = 0;
    
    int max_pixels = (int)sqrt(img->ancho * img->ancho + img->alto * img->alto) + 10;
    
    while (*num_pixels < max_pixels) {
        if (x >= 0 && x < img->ancho && y >= 0 && y < img->alto) {
            if (img->pixels[y][x] >= valor) {
                img->pixels[y][x] -= valor;
            } else {
                img->pixels[y][x] = 0;
            }
            (*num_pixels)++;
        }
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Calcula el beneficio de dibujar un hilo entre dos clavos
// Mayor beneficio = mas oscuridad falta por cubrir en esa trayectoria
double calcular_error_hilo(Imagen *original, Imagen *actual, Punto *clavos, int clavo1, int clavo2) {
    int x1 = (int)round(clavos[clavo1].x);
    int y1 = (int)round(clavos[clavo1].y);
    int x2 = (int)round(clavos[clavo2].x);
    int y2 = (int)round(clavos[clavo2].y);
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1;
    int y = y1;
    double beneficio_total = 0.0;
    int max_iteraciones = (int)sqrt(original->ancho * original->ancho + original->alto * original->alto) + 10;
    int iteraciones = 0;
    
    while (iteraciones < max_iteraciones) {
        if (x >= 0 && x < original->ancho && y >= 0 && y < original->alto) {
            int oscuridad_original = 255 - original->pixels[y][x];
            int oscuridad_actual = 255 - actual->pixels[y][x];
            int oscuridad_faltante = oscuridad_original - oscuridad_actual;
            if (oscuridad_faltante > 0) {
                beneficio_total += oscuridad_faltante;
            }
        }
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
        
        iteraciones++;
    }
    
    return beneficio_total;
}

// Calcula el error total entre dos imagenes (suma de diferencias al cuadrado)
double calcular_error_total(Imagen *original, Imagen *actual) {
    if (original->ancho != actual->ancho || original->alto != actual->alto) {
        return -1.0;
    }
    
    double error_total = 0.0;
    for (int i = 0; i < original->alto; i++) {
        for (int j = 0; j < original->ancho; j++) {
            int diff = abs((int)original->pixels[i][j] - (int)actual->pixels[i][j]);
            error_total += diff * diff;
        }
    }
    
    return error_total;
}

// Funcion de comparacion para qsort (ordena hilos por beneficio descendente)
int comparar_hilos(const void *a, const void *b) {
    Hilo *hilo_a = (Hilo*)a;
    Hilo *hilo_b = (Hilo*)b;
    
    if (hilo_a->error > hilo_b->error) return -1;
    if (hilo_a->error < hilo_b->error) return 1;
    return 0;
}

// Selecciona p hilos candidatos aleatorios y calcula su beneficio
void seleccionar_hilos_candidatos(Hilo *candidatos, int p, int n, Punto *clavos, 
                                  Imagen *original, Imagen *actual, int *usados, int num_usados) {
    int count = 0;
    int intentos = 0;
    const int max_intentos = p * 10;
    
    while (count < p && intentos < max_intentos) {
        intentos++;
        int clavo1 = rand() % n;
        int clavo2 = rand() % n;
        
        if (clavo1 == clavo2) continue;
        
        candidatos[count].clavo1 = clavo1;
        candidatos[count].clavo2 = clavo2;
        candidatos[count].error = calcular_error_hilo(original, actual, clavos, clavo1, clavo2);
        candidatos[count].puntuacion = candidatos[count].error;
        
        count++;
    }
}

// Algoritmo voraz principal para generar la imagen con hilos
// En cada iteracion selecciona los s mejores hilos de p candidatos aleatorios
void algoritmo_voraz(Imagen *original, Imagen *resultado, Punto *clavos, 
                     Parametros *params, int *hilos_seleccionados, int *num_hilos,
                     double *error_final) {
    for (int i = 0; i < resultado->alto; i++) {
        for (int j = 0; j < resultado->ancho; j++) {
            resultado->pixels[i][j] = 255;
        }
    }
    
    int max_hilos = 50000;
    int *hilos_usados = (int*)malloc(max_hilos * 2 * sizeof(int));
    if (hilos_usados == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para hilos usados\n");
        return;
    }
    
    Hilo *candidatos = (Hilo*)malloc(params->p * sizeof(Hilo));
    if (candidatos == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para candidatos\n");
        free(hilos_usados);
        return;
    }
    
    *num_hilos = 0;
    int iteraciones_sin_mejora = 0;
    const int max_iteraciones_sin_mejora = 20;
    
    double error_anterior = calcular_error_total(original, resultado);
    
    printf("Iniciando algoritmo voraz...\n");
    printf("Error inicial: %.2f\n", error_anterior);
    
    while (*num_hilos < max_hilos) {
        seleccionar_hilos_candidatos(candidatos, params->p, params->n, clavos, 
                                     original, resultado, hilos_usados, *num_hilos);
        
        qsort(candidatos, params->p, sizeof(Hilo), comparar_hilos);
        
        int hilos_a_dibujar = (params->s < params->p) ? params->s : params->p;
        
        bool mejora_encontrada = false;
        
        for (int i = 0; i < hilos_a_dibujar; i++) {
            int clavo1 = candidatos[i].clavo1;
            int clavo2 = candidatos[i].clavo2;
            
            int num_pixels;
            dibujar_linea_bresenham(resultado,
                                    (int)round(clavos[clavo1].x),
                                    (int)round(clavos[clavo1].y),
                                    (int)round(clavos[clavo2].x),
                                    (int)round(clavos[clavo2].y),
                                    5,
                                    &num_pixels);
            
            hilos_seleccionados[*num_hilos * 2] = clavo1;
            hilos_seleccionados[*num_hilos * 2 + 1] = clavo2;
            (*num_hilos)++;
            
            mejora_encontrada = true;
        }
        
        double error_actual = 0.0;
        bool error_calculado = false;
        
        int num_iteraciones = *num_hilos / params->s;
        if (num_iteraciones % 30 == 0 || num_iteraciones < 5) {
            error_actual = calcular_error_total(original, resultado);
            error_calculado = true;
            
            if (error_calculado) {
                if (error_actual < error_anterior) {
                    error_anterior = error_actual;
                    iteraciones_sin_mejora = 0;
                } else {
                    iteraciones_sin_mejora++;
                }
            }
        }
        
        if (iteraciones_sin_mejora >= max_iteraciones_sin_mejora) {
            printf("Parada: No hay mejora en %d iteraciones\n", max_iteraciones_sin_mejora);
            fflush(stdout);
            break;
        }
        
        if (!mejora_encontrada) {
            printf("Parada: No se encontraron hilos utiles\n");
            fflush(stdout);
            break;
        }
        
        if (*num_hilos % 500 == 0) {
            double error_mostrar = calcular_error_total(original, resultado);
            printf("Hilos dibujados: %d, Error actual: %.2f\n", *num_hilos, error_mostrar);
            fflush(stdout);
        }
    }
    
    printf("Calculando error final...\n");
    fflush(stdout);
    
    *error_final = calcular_error_total(original, resultado);
    
    printf("Algoritmo voraz completado. Hilos dibujados: %d, Error final: %.2f\n", *num_hilos, *error_final);
    fflush(stdout);
    
    if (candidatos != NULL) {
        free(candidatos);
    }
    if (hilos_usados != NULL) {
        free(hilos_usados);
    }
    
    printf("Memoria liberada correctamente\n");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Uso: %s <n> <p> <s> <imagen_entrada> <imagen_salida>\n", argv[0]);
        fprintf(stderr, "  n: numero de clavos\n");
        fprintf(stderr, "  p: numero de hilos candidatos por iteracion\n");
        fprintf(stderr, "  s: numero de mejores hilos a seleccionar\n");
        fprintf(stderr, "  imagen_entrada: archivo PGM de entrada\n");
        fprintf(stderr, "  imagen_salida: archivo PGM de salida\n");
        return 1;
    }
    
    Parametros params;
    params.n = atoi(argv[1]);
    params.p = atoi(argv[2]);
    params.s = atoi(argv[3]);
    
    if (params.n <= 0 || params.p <= 0 || params.s <= 0) {
        fprintf(stderr, "Error: Los parametros deben ser positivos\n");
        return 1;
    }
    
    if (params.s > params.p) {
        fprintf(stderr, "Error: s no puede ser mayor que p\n");
        return 1;
    }
    
    srand(time(NULL));
    
    clock_t inicio = clock();
    
    printf("Leyendo imagen: %s\n", argv[4]);
    Imagen *original = leer_pgm(argv[4]);
    if (original == NULL) {
        return 1;
    }
    
    printf("Imagen cargada: %d x %d pixeles\n", original->ancho, original->alto);
    
    Imagen *resultado = (Imagen*)malloc(sizeof(Imagen));
    if (resultado == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para imagen resultado\n");
        liberar_imagen(original);
        return 1;
    }
    
    resultado->ancho = original->ancho;
    resultado->alto = original->alto;
    resultado->pixels = (unsigned char**)malloc(resultado->alto * sizeof(unsigned char*));
    if (resultado->pixels == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para pixeles resultado\n");
        free(resultado);
        liberar_imagen(original);
        return 1;
    }
    
    for (int i = 0; i < resultado->alto; i++) {
        resultado->pixels[i] = (unsigned char*)malloc(resultado->ancho * sizeof(unsigned char));
        if (resultado->pixels[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(resultado->pixels[j]);
            }
            free(resultado->pixels);
            free(resultado);
            liberar_imagen(original);
            return 1;
        }
    }
    
    printf("Generando %d clavos en el borde de la imagen...\n", params.n);
    Punto *clavos = generar_clavos(params.n, original->ancho, original->alto);
    if (clavos == NULL) {
        fprintf(stderr, "Error: No se pudo generar clavos\n");
        liberar_imagen(resultado);
        liberar_imagen(original);
        return 1;
    }
    
    int max_hilos = 50000;
    int *hilos_seleccionados = (int*)malloc(max_hilos * 2 * sizeof(int));
    if (hilos_seleccionados == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para hilos seleccionados\n");
        free(clavos);
        liberar_imagen(resultado);
        liberar_imagen(original);
        return 1;
    }
    
    int num_hilos = 0;
    double error_final = 0.0;
    
    printf("Ejecutando algoritmo voraz...\n");
    fflush(stdout);
    
    algoritmo_voraz(original, resultado, clavos, &params, 
                   hilos_seleccionados, &num_hilos, &error_final);
    
    clock_t fin = clock();
    double tiempo_ejecucion = ((double)(fin - inicio)) / CLOCKS_PER_SEC;
    
    printf("\nEscribiendo imagen resultado: %s\n", argv[5]);
    fflush(stdout);
    
    int escritura_exitosa = escribir_pgm(argv[5], resultado);
    
    if (!escritura_exitosa) {
        fprintf(stderr, "Error: No se pudo escribir imagen resultado\n");
        fflush(stderr);
    } else {
        printf("Imagen resultado escrita correctamente\n");
        fflush(stdout);
    }
    
    printf("\n=== ESTADISTICAS DE EJECUCION ===\n");
    fflush(stdout);
    printf("Parametros:\n");
    printf("  Numero de clavos (n): %d\n", params.n);
    printf("  Hilos candidatos por iteracion (p): %d\n", params.p);
    printf("  Mejores hilos seleccionados (s): %d\n", params.s);
    printf("Resultados:\n");
    printf("  Numero de hilos dibujados: %d\n", num_hilos);
    printf("  Error final: %.2f\n", error_final);
    printf("  Tiempo de ejecucion: %.2f segundos\n", tiempo_ejecucion);
    printf("===================================\n");
    fflush(stdout);
    
    free(hilos_seleccionados);
    free(clavos);
    liberar_imagen(resultado);
    liberar_imagen(original);
    
    return 0;
}
