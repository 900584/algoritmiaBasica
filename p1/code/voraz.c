#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Comprobar que se pasa el nombre del archivo
    if (argc < 2) {
        printf("Uso: %s <nombre_archivo.pgm>\n", argv[0]);
        return 1;
    }

    // Ruta base fija (ajústala si hace falta)
    const char *ruta_base = "imagenes/pixel/";

    // Construir la ruta completa: ruta_base + nombre_archivo
    char ruta_completa[512];
    snprintf(ruta_completa, sizeof(ruta_completa), "%s%s", ruta_base, argv[1]);

    // Abrir el archivo .pgm con la ruta completa
    FILE *archivo = fopen(ruta_completa, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo: %s\n", ruta_completa);
        return 1;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), archivo) != NULL) {
        printf("%s", buffer);
    }
    fclose(archivo);
    return 0;
}