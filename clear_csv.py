# -*- coding: utf-8 -*-
"""
Created on Mon May 13 13:12:46 2024

@author: LuisFco
"""

import csv
import tkinter as tk
from tkinter import filedialog
import os

def limpieza_csv(archivo_entrada, archivo_salida):
    with open(archivo_entrada, 'r', newline='') as archivo_entrada:
        lector_csv = csv.reader(archivo_entrada)
        datos = list(lector_csv)[40:]  # Excluir las primeras 40 filas
    # Excluir la segunda fila restante
    if len(datos) > 1:
        datos.pop(1)
    
    with open(archivo_salida, 'w', newline='') as archivo_salida:
        escritor_csv = csv.writer(archivo_salida)
        for i, fila in enumerate(datos):
            fila_limpia = [
                celda.replace("L", "").replace("+", "").replace(" ", "") if index != 1 else celda.replace("L", "").replace("+", "")
                for index, celda in enumerate(fila)
            ]
            if i > 0:  # A partir de la segunda fila
                fila_limpia = fila_limpia[:-3]  # Eliminar las últimas tres columnas
            escritor_csv.writerow(fila_limpia)

    print("Se han eliminado las primeras 38 filas, los caracteres 'L', espacios, '+' y las tres últimas columnas del archivo CSV exitosamente.")
    return archivo_salida

def seleccionar_archivo():
    archivo_entrada = filedialog.askopenfilename(title="Seleccione el archivo CSV de entrada")
    if archivo_entrada:
        nombre_archivo = os.path.basename(archivo_entrada)
        archivo_seleccionado_label.config(text=f"Archivo seleccionado: {nombre_archivo}")

        archivo_salida = filedialog.asksaveasfilename(title="Seleccione el archivo CSV de salida", defaultextension=".csv")
        if archivo_salida:
            nombre_archivo_guardado = os.path.basename(archivo_salida)
            archivo_guardado_label.config(text=f"Archivo guardado: {nombre_archivo_guardado}")
            archivo_guardado_label.pack(pady=5)

            archivo_salida = limpieza_csv(archivo_entrada, archivo_salida)


# Configuración de la ventana Tkinter
root = tk.Tk()
root.title("Limpieza de archivo CSV")

# Determinar el ancho y la altura de la pantalla
ancho_pantalla = root.winfo_screenwidth()
altura_pantalla = root.winfo_screenheight()

# Tamaño y posición de la ventana
ancho_ventana = 450
altura_ventana = 300
pos_x = (ancho_pantalla - ancho_ventana) // 2
pos_y = (altura_pantalla - altura_ventana) // 2
root.geometry(f"{ancho_ventana}x{altura_ventana}+{pos_x}+{pos_y}")  # Tamaño y posición centrada de la ventana

# Etiqueta para mostrar el archivo seleccionado
archivo_seleccionado_label = tk.Label(root, text="", font=("Arial", 12))
archivo_seleccionado_label.pack(pady=5)

# Etiqueta para mostrar el archivo guardado
archivo_guardado_label = tk.Label(root, text="", font=("Arial", 12))
archivo_guardado_label.pack(pady=5)

# Botón para seleccionar archivo
boton_seleccionar = tk.Button(root, text="Seleccionar archivo", command=seleccionar_archivo)
boton_seleccionar.pack(pady=(60, 20))  # Centra el botón tanto horizontal como verticalmente

root.mainloop()