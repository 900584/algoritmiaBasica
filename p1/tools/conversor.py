import cv2
import os
import sys

def png_to_pgm(input_path, output_path):
    """Convierte PNG a PGM (formato ASCII P2)"""
    img_gray = cv2.imread(input_path, 0)
    
    if img_gray is None:
        print(f"Error: No se pudo leer {input_path}")
        return False
    
    # Guardar como PGM ASCII (P2) - legible por humanos
    with open(output_path, 'w') as f:
        f.write(f"P2\n{img_gray.shape[1]} {img_gray.shape[0]}\n255\n")
        for row in img_gray:
            f.write(' '.join(map(str, row)) + '\n')
    
    print(f"PNG → PGM")
    print(f"  Entrada: {input_path}")
    print(f"  Salida:  {output_path}")
    print(f"  Tamaño:  {img_gray.shape}")
    return True

def pgm_to_png(input_path, output_path):
    """Convierte PGM a PNG"""
    img_gray = cv2.imread(input_path, cv2.IMREAD_GRAYSCALE)
    
    if img_gray is None:
        print(f"Error: No se pudo leer {input_path}")
        return False
    
    cv2.imwrite(output_path, img_gray)
    
    print(f"PGM → PNG")
    print(f"  Entrada: {input_path}")
    print(f"  Salida:  {output_path}")
    print(f"  Tamaño:  {img_gray.shape}")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python conversor.py <filename.png|filename.pgm>")
        sys.exit(1)
    
    # Obtener el nombre del archivo (sin ruta)
    filename = os.path.basename(sys.argv[1])
    base_name, ext = os.path.splitext(filename)
    ext = ext.lower()
    
    # Obtener directorio base del script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Construir rutas automáticas según la extensión
    if ext == ".png":
        # PNG → PGM: de raw/ a pixel/
        input_file = os.path.join(script_dir, "..", "imagenes", "raw", filename)
        output_file = os.path.join(script_dir, "..", "imagenes", "pixel", base_name + ".pgm")
    elif ext == ".pgm":
        # PGM → PNG: de pixel/ a raw/
        input_file = os.path.join(script_dir, "..", "imagenes", "pixel", filename)
        output_file = os.path.join(script_dir, "..", "imagenes", "raw", base_name + "_gray.png")
    else:
        print(f"Error: Extensión no soportada '{ext}'. Usa .png o .pgm")
        sys.exit(1)
    
    # Comprobar si existe el archivo de entrada
    if not os.path.exists(input_file):
        print(f"Error: El archivo no existe: {input_file}")
        sys.exit(1)
    
    # Convertir según la extensión
    if ext == ".png":
        success = png_to_pgm(input_file, output_file)
    elif ext == ".pgm":
        success = pgm_to_png(input_file, output_file)
    
    if success:
        print("Conversión exitosa")
    else:
        print("Error en la conversión")
        sys.exit(1)