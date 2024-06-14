import os
import sys
from tkinter import Tk, filedialog
from Prueba_limpieza import limpieza_csv
from Prueba_array import process_array
from training_data import training
from plottings import plotting

def main():
    # Inicializar la instancia de Tkinter para usar filedialog sin mostrar la ventana principal
    root = Tk()
    root.withdraw()  # Ocultar la ventana principal de Tkinter

    # Solicitar el archivo inicial
    initial_file = filedialog.askopenfilename(title="Seleccione el archivo de datos")
    if not initial_file:
        print("No se seleccionó ningún archivo de datos. Saliendo...")
        return

    # Solicitar el archivo extra
    extra_file = filedialog.askopenfilename(title="Seleccione el archivo de meteo")
    if not extra_file:
        print("No se seleccionó ningún archivo de meteo. Saliendo...")
        return

    # Solicitar la carpeta de destino
    output_folder = filedialog.askdirectory(title="Seleccione la carpeta de destino para los archivos generados")
    if not output_folder:
        print("No se seleccionó ninguna carpeta de destino. Saliendo...")
        return

    # Destruir la ventana de Tk después de seleccionar los archivos
    root.destroy()

    # Obtener el nombre base y extensión del archivo inicial
    initial_filename = os.path.basename(initial_file)
    base_name, ext = os.path.splitext(initial_filename)

    # Definir nombres de los archivos procesados en la carpeta de destino seleccionada
    arch1 = os.path.join(output_folder, f"{base_name}_clear_csv{ext}")
    arch2 = os.path.join(output_folder, f"{base_name}_Prueba_array{ext}")
    arch3 = os.path.join(output_folder, f"{base_name}_training_data{ext}")
    arch4 = os.path.join(output_folder, f"{base_name}_plottings{ext}")

    # Ejecutar las funciones importadas
    limpieza_csv(initial_file, arch1)
    process_array(arch1, arch2)
    training(arch2, extra_file, arch3)
    plotting(arch3, arch4)

    print("Proceso completado")

if __name__ == "__main__":
    main()
