import cv2
import numpy as np
import os
import sys

def _imread_unicode(path, flags=cv2.IMREAD_GRAYSCALE):
    """Lee una imagen desde una ruta con caracteres Unicode (ej. tildes en Windows)."""
    with open(path, "rb") as f:
        buf = np.frombuffer(f.read(), dtype=np.uint8)
    return cv2.imdecode(buf, flags)

def png_to_pgm(input_path, output_path):
    """Convierte PNG a PGM (formato ASCII P2)"""
    img_gray = _imread_unicode(input_path, cv2.IMREAD_GRAYSCALE)

    if img_gray is None:
        print(f"Error: No se pudo leer {input_path}")
        return False

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w') as f:
        f.write(f"P2\n{img_gray.shape[1]} {img_gray.shape[0]}\n255\n")
        for row in img_gray:
            f.write(' '.join(map(str, row)) + '\n')

    print(f"PNG -> PGM")
    print(f"  Entrada: {input_path}")
    print(f"  Salida:  {output_path}")
    print(f"  Tamanyo: {img_gray.shape[1]}x{img_gray.shape[0]} px")
    return True

def pgm_to_png(input_path, output_path):
    """Convierte PGM a PNG"""
    img_gray = _imread_unicode(input_path, cv2.IMREAD_GRAYSCALE)

    if img_gray is None:
        print(f"Error: No se pudo leer {input_path}")
        return False

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    _, buf = cv2.imencode(".png", img_gray)
    with open(output_path, "wb") as f:
        f.write(buf.tobytes())

    print(f"PGM -> PNG")
    print(f"  Entrada: {input_path}")
    print(f"  Salida:  {output_path}")
    print(f"  Tamanyo: {img_gray.shape[1]}x{img_gray.shape[0]} px")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python tools/conversor.py <ruta/a/imagen.png|.pgm>")
        print("")
        print("  La ruta es relativa al directorio desde donde ejecutas el script.")
        print("  El resultado se guarda en conversions/pgm/ o conversions/png/")
        print("")
        print("  Ejemplos:")
        print("    python tools/conversor.py imagenes/raw/messi.png")
        print("    python tools/conversor.py pruebas/messi.pgm")
        sys.exit(1)

    input_path = sys.argv[1]

    if not os.path.exists(input_path):
        print(f"Error: No se encuentra el archivo '{input_path}'")
        sys.exit(1)

    base_name = os.path.splitext(os.path.basename(input_path))[0]
    ext = os.path.splitext(input_path)[1].lower()

    if ext == ".png":
        output_path = os.path.join("conversiones", "pgm", base_name + ".pgm")
        success = png_to_pgm(input_path, output_path)
    elif ext == ".pgm":
        output_path = os.path.join("conversiones", "png", base_name + ".png")
        success = pgm_to_png(input_path, output_path)
    else:
        print(f"Error: Extension no soportada '{ext}'. Usa .png o .pgm")
        sys.exit(1)

    if success:
        print("Conversion exitosa")
    else:
        print("Error en la conversion")
        sys.exit(1)
