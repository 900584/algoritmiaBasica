#define _USE_MATH_DEFINES  // Para Windows: define M_PI y otras constantes matemáticas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Estructura para representar un punto 2D
typedef struct {
    double x, y;
} Punto;

// Estructura para representar un hilo (par de clavos)
typedef struct {
    int clavo1, clavo2;  // Índices de los clavos
    double error;        // Error que reduciría este hilo
    double puntuacion;   // Puntuación para selección
} Hilo;

// Estructura para representar una imagen en escala de grises
typedef struct {
    int ancho, alto;
    unsigned char **pixels;  // Matriz de píxeles [alto][ancho]
} Imagen;

// Estructura para almacenar parámetros del programa
typedef struct {
    int n;  // Número de clavos
    int p;  // Número de hilos candidatos por iteración
    int s;  // Número de mejores hilos a seleccionar
} Parametros;

// Función para leer una imagen PGM (formato P2 - ASCII)
Imagen* leer_pgm(const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", nombre_archivo);
        return NULL;
    }
    
    char formato[3];
    if (fscanf(archivo, "%2s", formato) != 1 || strcmp(formato, "P2") != 0) {
        fprintf(stderr, "Error: Formato PGM no válido (debe ser P2)\n");
        fclose(archivo);
        return NULL;
    }
    
    Imagen *img = (Imagen*)malloc(sizeof(Imagen));
    if (img == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        fclose(archivo);
        return NULL;
    }
    
    // Leer ancho, alto y valor máximo
    int max_valor;
    if (fscanf(archivo, "%d %d %d", &img->ancho, &img->alto, &max_valor) != 3) {
        fprintf(stderr, "Error: No se pudo leer dimensiones de la imagen\n");
        free(img);
        fclose(archivo);
        return NULL;
    }
    
    // Asignar memoria para la matriz de píxeles
    img->pixels = (unsigned char**)malloc(img->alto * sizeof(unsigned char*));
    if (img->pixels == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para píxeles\n");
        free(img);
        fclose(archivo);
        return NULL;
    }
    
    for (int i = 0; i < img->alto; i++) {
        img->pixels[i] = (unsigned char*)malloc(img->ancho * sizeof(unsigned char));
        if (img->pixels[i] == NULL) {
            fprintf(stderr, "Error: No se pudo asignar memoria para fila %d\n", i);
            // Liberar memoria ya asignada
            for (int j = 0; j < i; j++) {
                free(img->pixels[j]);
            }
            free(img->pixels);
            free(img);
            fclose(archivo);
            return NULL;
        }
    }
    
    // Leer valores de píxeles
    int valor;
    for (int i = 0; i < img->alto; i++) {
        for (int j = 0; j < img->ancho; j++) {
            if (fscanf(archivo, "%d", &valor) != 1) {
                fprintf(stderr, "Error: No se pudo leer píxel en (%d, %d)\n", j, i);
                // Liberar memoria
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

// Función para escribir una imagen PGM (formato P2 - ASCII)
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

// Función para liberar memoria de una imagen
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

// Función para crear una copia de una imagen
Imagen* copiar_imagen(Imagen *original) {
    if (original == NULL) return NULL;
    
    Imagen *copia = (Imagen*)malloc(sizeof(Imagen));
    if (copia == NULL) return NULL;
    
    copia->ancho = original->ancho;
    copia->alto = original->alto;
    
    copia->pixels = (unsigned char**)malloc(copia->alto * sizeof(unsigned char*));
    if (copia->pixels == NULL) {
        free(copia);
        return NULL;
    }
    
    for (int i = 0; i < copia->alto; i++) {
        copia->pixels[i] = (unsigned char*)malloc(copia->ancho * sizeof(unsigned char));
        if (copia->pixels[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(copia->pixels[j]);
            }
            free(copia->pixels);
            free(copia);
            return NULL;
        }
        memcpy(copia->pixels[i], original->pixels[i], copia->ancho * sizeof(unsigned char));
    }
    
    return copia;
}

// Función para generar clavos en un círculo alrededor de la imagen
// CORREGIDO: Radio ajustado para que los clavos estén más cerca de la imagen
// Esto permite que los hilos formen mejor la imagen sin ser demasiado tangenciales
Punto* generar_clavos(int n, int ancho, int alto) {
    Punto *clavos = (Punto*)malloc(n * sizeof(Punto));
    if (clavos == NULL) return NULL;
    
    // Centro de la imagen
    double centro_x = ancho / 2.0;
    double centro_y = alto / 2.0;
    
    // Radio del círculo: usar el menor de los lados dividido por 2
    // Esto hace que el círculo toque los bordes de la imagen
    // Los clavos estarán justo en el borde o ligeramente fuera
    double radio = (ancho < alto ? ancho : alto) / 2.0;
    
    // Generar clavos distribuidos uniformemente en el círculo
    for (int i = 0; i < n; i++) {
        double angulo = 2.0 * M_PI * i / n;
        clavos[i].x = centro_x + radio * cos(angulo);
        clavos[i].y = centro_y + radio * sin(angulo);
    }
    
    return clavos;
}

// Función auxiliar para obtener píxeles de una línea (no usada actualmente, pero útil para debugging)
int obtener_pixels_linea(Imagen *img, int x1, int y1, int x2, int y2, 
                         int *pixels_x, int *pixels_y, int max_pixels) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1;
    int y = y1;
    int count = 0;
    
    while (count < max_pixels) {
        // Verificar que el píxel está dentro de los límites
        if (x >= 0 && x < img->ancho && y >= 0 && y < img->alto) {
            pixels_x[count] = x;
            pixels_y[count] = y;
            count++;
        }
        
        // Si llegamos al punto final, salir
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
    
    return count;
}

// Función corregida de Bresenham (versión más robusta)
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
    
    // Calcular número máximo de píxeles (diagonal de la imagen)
    int max_pixels = (int)sqrt(img->ancho * img->ancho + img->alto * img->alto) + 10;
    
    while (*num_pixels < max_pixels) {
        // Verificar que el píxel está dentro de los límites
        if (x >= 0 && x < img->ancho && y >= 0 && y < img->alto) {
            // Actualizar píxel: siempre restar valor (permite acumulación)
            // Esto permite que múltiples hilos se superpongan y oscurezcan áreas
            if (img->pixels[y][x] >= valor) {
                img->pixels[y][x] -= valor;
            } else {
                img->pixels[y][x] = 0;  // No puede ser negativo
            }
            (*num_pixels)++;
        }
        
        // Si llegamos al punto final, salir
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

// Función para calcular el "beneficio" de un hilo
// CORREGIDO: Calcula la diferencia entre lo que falta por cubrir
// Compara la imagen original con la imagen actual (lo que ya se ha dibujado)
// El beneficio es la suma de (oscuridad_original - oscuridad_ya_cubierta)
// Si el original es oscuro (valor bajo) y aún no está cubierto (actual alto), el beneficio es máximo
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
        // Verificar que el píxel está dentro de los límites
        if (x >= 0 && x < original->ancho && y >= 0 && y < original->alto) {
            // Calcular oscuridad original (255 - valor_original)
            // Valor bajo en original = oscuro = necesita hilos
            int oscuridad_original = 255 - original->pixels[y][x];
            
            // Calcular oscuridad ya cubierta (255 - valor_actual)
            // Valor bajo en actual = ya está oscuro = ya cubierto
            int oscuridad_actual = 255 - actual->pixels[y][x];
            
            // El beneficio es cuánta oscuridad falta por cubrir
            // Si original es muy oscuro (oscuridad_original alto) y actual es claro (oscuridad_actual bajo)
            // entonces falta mucho por cubrir (beneficio alto)
            int oscuridad_faltante = oscuridad_original - oscuridad_actual;
            if (oscuridad_faltante > 0) {
                beneficio_total += oscuridad_faltante;
            }
        }
        
        // Si llegamos al punto final, salir
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

// Función para calcular el error total de la imagen (diferencia con imagen original)
// CORREGIDO: Compara directamente original con actual
// En string art: donde original es oscuro (valor bajo), resultado debería ser oscuro (valor bajo)
// Donde original es claro (valor alto), resultado debería ser claro (valor alto)
double calcular_error_total(Imagen *original, Imagen *actual) {
    if (original->ancho != actual->ancho || original->alto != actual->alto) {
        return -1.0;
    }
    
    double error_total = 0.0;
    for (int i = 0; i < original->alto; i++) {
        for (int j = 0; j < original->ancho; j++) {
            // Calcular diferencia: cuanto más diferente, mayor error
            int diff = abs((int)original->pixels[i][j] - (int)actual->pixels[i][j]);
            error_total += diff * diff;  // Suma de cuadrados de diferencias
        }
    }
    
    return error_total;
}

// Función de comparación para ordenar hilos por error (mayor error primero)
int comparar_hilos(const void *a, const void *b) {
    Hilo *hilo_a = (Hilo*)a;
    Hilo *hilo_b = (Hilo*)b;
    
    if (hilo_a->error > hilo_b->error) return -1;
    if (hilo_a->error < hilo_b->error) return 1;
    return 0;
}

// Función para seleccionar p hilos candidatos aleatorios
// CORREGIDO: Ahora recibe original y actual para calcular el beneficio correctamente
void seleccionar_hilos_candidatos(Hilo *candidatos, int p, int n, Punto *clavos, 
                                  Imagen *original, Imagen *actual, int *usados, int num_usados) {
    int count = 0;
    int intentos = 0;
    const int max_intentos = p * 10;  // Límite de intentos para evitar bucles infinitos
    
    // Estrategia: seleccionar aleatoriamente parejas de clavos
    while (count < p && intentos < max_intentos) {
        intentos++;
        int clavo1 = rand() % n;
        int clavo2 = rand() % n;
        
        // Asegurar que son diferentes
        if (clavo1 == clavo2) continue;
        
        // Verificar que este hilo no está ya usado (opcional, puede permitir repeticiones)
        // Comentado para permitir más flexibilidad
        /*
        bool ya_usado = false;
        for (int i = 0; i < num_usados; i++) {
            if ((usados[i*2] == clavo1 && usados[i*2+1] == clavo2) ||
                (usados[i*2] == clavo2 && usados[i*2+1] == clavo1)) {
                ya_usado = true;
                break;
            }
        }
        if (ya_usado) continue;
        */
        
        // Calcular beneficio de este hilo comparando original vs actual
        // Mayor beneficio = más oscuridad falta por cubrir en esa trayectoria
        candidatos[count].clavo1 = clavo1;
        candidatos[count].clavo2 = clavo2;
        candidatos[count].error = calcular_error_hilo(original, actual, clavos, clavo1, clavo2);
        candidatos[count].puntuacion = candidatos[count].error;
        
        count++;
    }
}

// Función principal del algoritmo voraz
void algoritmo_voraz(Imagen *original, Imagen *resultado, Punto *clavos, 
                     Parametros *params, int *hilos_seleccionados, int *num_hilos,
                     double *error_final) {
    // Inicializar imagen resultado (blanco = 255, fondo claro)
    for (int i = 0; i < resultado->alto; i++) {
        for (int j = 0; j < resultado->ancho; j++) {
            resultado->pixels[i][j] = 255;
        }
    }
    
    // CORREGIDO: Ya no necesitamos "trabajo" como copia de original
    // Usamos directamente "resultado" para calcular qué falta por cubrir
    // El beneficio se calcula comparando original con resultado
    
    // Array para almacenar hilos usados (opcional, para evitar repeticiones exactas)
    int max_hilos = 50000;  // Límite máximo de hilos (aumentado para permitir más iteraciones)
    int *hilos_usados = (int*)malloc(max_hilos * 2 * sizeof(int));
    if (hilos_usados == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para hilos usados\n");
        return;
    }
    
    // Array para candidatos
    Hilo *candidatos = (Hilo*)malloc(params->p * sizeof(Hilo));
    if (candidatos == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para candidatos\n");
        free(hilos_usados);
        return;
    }
    
    *num_hilos = 0;
    int iteraciones_sin_mejora = 0;
    const int max_iteraciones_sin_mejora = 20;  // Iteraciones sin mejora antes de parar
    
    // Calcular error inicial comparando resultado (blanco) con original
    // CORREGIDO: No necesitamos invertir, comparamos directamente
    // Resultado empieza en blanco (255), original tiene valores variados
    // El error inicial es alto porque resultado es todo blanco y original tiene áreas oscuras
    double error_anterior = calcular_error_total(original, resultado);
    
    printf("Iniciando algoritmo voraz...\n");
    printf("Error inicial: %.2f\n", error_anterior);
    
    while (*num_hilos < max_hilos) {
        // Seleccionar p hilos candidatos
        // CORREGIDO: Pasamos original y resultado (actual) para calcular beneficio correctamente
        seleccionar_hilos_candidatos(candidatos, params->p, params->n, clavos, 
                                     original, resultado, hilos_usados, *num_hilos);
        
        // Ordenar candidatos por beneficio (mayor beneficio = mejor, porque atraviesa más píxeles oscuros)
        qsort(candidatos, params->p, sizeof(Hilo), comparar_hilos);
        
        // Seleccionar los s mejores hilos
        int hilos_a_dibujar = (params->s < params->p) ? params->s : params->p;
        
        bool mejora_encontrada = false;
        
        for (int i = 0; i < hilos_a_dibujar; i++) {
            int clavo1 = candidatos[i].clavo1;
            int clavo2 = candidatos[i].clavo2;
            
            // Dibujar el hilo en la imagen resultado
            // IMPORTANTE: En string art, cada hilo oscurece ligeramente los píxeles
            // La superposición de múltiples hilos crea áreas más oscuras
            // Restamos un valor moderado (5) para permitir acumulación efectiva
            // Con 5 por hilo, necesitamos ~50 hilos superpuestos para llegar a negro
            int num_pixels;
            dibujar_linea_bresenham(resultado,
                                    (int)round(clavos[clavo1].x),
                                    (int)round(clavos[clavo1].y),
                                    (int)round(clavos[clavo2].x),
                                    (int)round(clavos[clavo2].y),
                                    5,  // Restar 5 de cada píxel (acumulación más efectiva)
                                    &num_pixels);
            
            // Guardar hilo seleccionado
            hilos_seleccionados[*num_hilos * 2] = clavo1;
            hilos_seleccionados[*num_hilos * 2 + 1] = clavo2;
            (*num_hilos)++;
            
            mejora_encontrada = true;
        }
        
        // Calcular error comparando directamente la imagen resultado con la original
        // CORREGIDO: No necesitamos invertir, comparamos directamente
        // Donde original es oscuro (valor bajo), resultado debería ser oscuro (valor bajo)
        // Donde original es claro (valor alto), resultado debería ser claro (valor alto)
        // OPTIMIZADO: Solo calcular cada 30 iteraciones para mejorar rendimiento
        double error_actual = 0.0;
        bool error_calculado = false;
        
        // Solo calcular error completo cada 30 iteraciones para optimizar (cada ~900 hilos con s=30)
        // O siempre en las primeras 5 iteraciones para tener referencia inicial
        int num_iteraciones = *num_hilos / params->s;
        if (num_iteraciones % 30 == 0 || num_iteraciones < 5) {
            // Comparar directamente sin invertir
            error_actual = calcular_error_total(original, resultado);
            error_calculado = true;
            
            // Verificar si hay mejora (error menor = mejor)
            if (error_calculado) {
                if (error_actual < error_anterior) {
                    error_anterior = error_actual;
                    iteraciones_sin_mejora = 0;
                } else {
                    iteraciones_sin_mejora++;
                }
            }
        }
        // En iteraciones intermedias, NO resetear el contador
        // Solo incrementamos cuando realmente calculamos el error y no mejora
        
        // Condiciones de parada
        if (iteraciones_sin_mejora >= max_iteraciones_sin_mejora) {
            printf("Parada: No hay mejora en %d iteraciones\n", max_iteraciones_sin_mejora);
            fflush(stdout);  // Asegurar que se muestre el mensaje
            break;
        }
        
        if (!mejora_encontrada) {
            printf("Parada: No se encontraron hilos útiles\n");
            fflush(stdout);  // Asegurar que se muestre el mensaje
            break;
        }
        
        // Mostrar progreso cada 500 hilos (para no saturar la salida)
        if (*num_hilos % 500 == 0) {
            // Calcular error para mostrar (solo cuando mostramos progreso)
            // CORREGIDO: Comparar directamente sin invertir
            double error_mostrar = calcular_error_total(original, resultado);
            printf("Hilos dibujados: %d, Error actual: %.2f\n", *num_hilos, error_mostrar);
            fflush(stdout);
        }
    }
    
    // Calcular error final comparando original con resultado
    // CORREGIDO: Comparar directamente sin invertir
    // Donde original es oscuro (valor bajo), resultado debería ser oscuro (valor bajo)
    // Donde original es claro (valor alto), resultado debería ser claro (valor alto)
    printf("Calculando error final...\n");
    fflush(stdout);
    
    *error_final = calcular_error_total(original, resultado);
    
    printf("Algoritmo voraz completado. Hilos dibujados: %d, Error final: %.2f\n", *num_hilos, *error_final);
    fflush(stdout);
    
    // Liberar memoria
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
    // Verificar argumentos
    if (argc < 6) {
        fprintf(stderr, "Uso: %s <n> <p> <s> <imagen_entrada> <imagen_salida>\n", argv[0]);
        fprintf(stderr, "  n: número de clavos\n");
        fprintf(stderr, "  p: número de hilos candidatos por iteración\n");
        fprintf(stderr, "  s: número de mejores hilos a seleccionar\n");
        fprintf(stderr, "  imagen_entrada: archivo PGM de entrada\n");
        fprintf(stderr, "  imagen_salida: archivo PGM de salida\n");
        return 1;
    }
    
    // Leer parámetros
    Parametros params;
    params.n = atoi(argv[1]);
    params.p = atoi(argv[2]);
    params.s = atoi(argv[3]);
    
    if (params.n <= 0 || params.p <= 0 || params.s <= 0) {
        fprintf(stderr, "Error: Los parámetros deben ser positivos\n");
        return 1;
    }
    
    if (params.s > params.p) {
        fprintf(stderr, "Error: s no puede ser mayor que p\n");
        return 1;
    }
    
    // Inicializar generador de números aleatorios
    srand(time(NULL));
    
    // Medir tiempo de inicio
    clock_t inicio = clock();
    
    // Leer imagen original
    printf("Leyendo imagen: %s\n", argv[4]);
    Imagen *original = leer_pgm(argv[4]);
    if (original == NULL) {
        return 1;
    }
    
    printf("Imagen cargada: %d x %d píxeles\n", original->ancho, original->alto);
    
    // Crear imagen resultado
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
        fprintf(stderr, "Error: No se pudo asignar memoria para píxeles resultado\n");
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
    
    // Generar clavos
    printf("Generando %d clavos en círculo...\n", params.n);
    Punto *clavos = generar_clavos(params.n, original->ancho, original->alto);
    if (clavos == NULL) {
        fprintf(stderr, "Error: No se pudo generar clavos\n");
        liberar_imagen(resultado);
        liberar_imagen(original);
        return 1;
    }
    
    // Array para almacenar hilos seleccionados
    // IMPORTANTE: Debe coincidir con max_hilos en algoritmo_voraz
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
    
    // Ejecutar algoritmo voraz
    printf("Ejecutando algoritmo voraz...\n");
    fflush(stdout);
    
    algoritmo_voraz(original, resultado, clavos, &params, 
                   hilos_seleccionados, &num_hilos, &error_final);
    
    printf("\n[DEBUG] Regresando de algoritmo_voraz\n");
    fflush(stdout);
    
    // Medir tiempo de fin
    clock_t fin = clock();
    double tiempo_ejecucion = ((double)(fin - inicio)) / CLOCKS_PER_SEC;
    
    printf("[DEBUG] Tiempo medido: %.2f segundos\n", tiempo_ejecucion);
    fflush(stdout);
    
    // Escribir imagen resultado
    printf("\nEscribiendo imagen resultado: %s\n", argv[5]);
    fflush(stdout);
    
    int escritura_exitosa = escribir_pgm(argv[5], resultado);
    
    printf("[DEBUG] Resultado de escribir_pgm: %d\n", escritura_exitosa);
    fflush(stdout);
    
    if (!escritura_exitosa) {
        fprintf(stderr, "Error: No se pudo escribir imagen resultado\n");
        fflush(stderr);
    } else {
        printf("Imagen resultado escrita correctamente\n");
        fflush(stdout);
    }
    
    // Mostrar estadísticas
    printf("\n=== ESTADÍSTICAS DE EJECUCIÓN ===\n");
    fflush(stdout);
    printf("Parámetros:\n");
    printf("  Número de clavos (n): %d\n", params.n);
    printf("  Hilos candidatos por iteración (p): %d\n", params.p);
    printf("  Mejores hilos seleccionados (s): %d\n", params.s);
    printf("Resultados:\n");
    printf("  Número de hilos dibujados: %d\n", num_hilos);
    printf("  Error final: %.2f\n", error_final);
    printf("  Tiempo de ejecución: %.2f segundos\n", tiempo_ejecucion);
    printf("===================================\n");
    fflush(stdout);
    
    // Liberar memoria
    free(hilos_seleccionados);
    free(clavos);
    liberar_imagen(resultado);
    liberar_imagen(original);
    
    return 0;
}
