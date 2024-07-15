# -*- coding: utf-8 -*-
"""
Created on Tue Apr  2 09:55:45 2024

@author: LuisFco
"""

import csv
import sys

def limpieza_csv(archivo_entrada, archivo_salida):
    # Open the input file and read the contents, skipping the first 40 rows
    with open(archivo_entrada, 'r', newline='') as archivo_entrada:
        lector_csv = csv.reader(archivo_entrada)
        datos = list(lector_csv)[40:]
        
    # Exclude the second row from the remaining data
    if len(datos) > 1:
        datos.pop(1)
    
    # Open the output file and write the cleaned data
    with open(archivo_salida, 'w', newline='') as archivo_salida:
        escritor_csv = csv.writer(archivo_salida)
        for i, fila in enumerate(datos):
            fila_limpia = [
                celda.replace("L", "").replace("+", "").replace(" ", "") if index != 1 else celda.replace("L", "").replace("+", "")
                for index, celda in enumerate(fila)
            ]
            if i > 0:  # Starting from the second row
                fila_limpia = fila_limpia[:-3]  # Remove the last three columns
            escritor_csv.writerow(fila_limpia)

    print("Se han eliminado las primeras 38 filas, los caracteres 'L', espacios, '+' y las tres últimas columnas del archivo CSV.")

def main():
    if len(sys.argv) != 3:
        print("Uso: python clear_csv.py <input_file> <output_file>")
        sys.exit(1)
    archivo_entrada = sys.argv[1]
    archivo_salida = sys.argv[2]
    limpieza_csv(archivo_entrada, archivo_salida)
    print(f"El archivo limpio se ha guardado en: {archivo_salida}")

if __name__ == "__main__":
    main()












