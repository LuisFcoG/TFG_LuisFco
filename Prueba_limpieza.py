# -*- coding: utf-8 -*-
"""
Created on Tue Apr  2 09:55:45 2024

@author: LuisFco
"""

import csv
import tkinter as tk
from tkinter import filedialog

def eliminar_primeras_filas(archivo_entrada, archivo_salida):
    with open(archivo_entrada, 'r', newline='') as archivo_entrada:
        lector_csv = csv.reader(archivo_entrada)
        datos = list(lector_csv)[38:]  # Excluir las primeras 38 filas
    
    with open(archivo_salida, 'w', newline='') as archivo_salida:
        escritor_csv = csv.writer(archivo_salida)
        escritor_csv.writerows(datos)
    print("Las primeras 38 filas se han eliminado del archivo CSV exitosamente.")

def seleccionar_archivo():
    archivo_entrada = filedialog.askopenfilename(title="Seleccione el archivo CSV de entrada")
    if archivo_entrada:
        archivo_salida = filedialog.asksaveasfilename(title="Seleccione el archivo CSV de salida", defaultextension=".csv")
        if archivo_salida:
            eliminar_primeras_filas(archivo_entrada, archivo_salida)

# Configuración de la ventana Tkinter
root = tk.Tk()
root.title("Eliminar primeras filas de archivo CSV")

# Botón para seleccionar archivo
boton_seleccionar = tk.Button(root, text="Seleccionar archivo", command=seleccionar_archivo)
boton_seleccionar.pack(pady=20)

root.mainloop()

