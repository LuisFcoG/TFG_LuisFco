# -*- coding: utf-8 -*-
"""
Created on Fri Jun 14 10:17:08 2024

@author: Luis Francisco
"""

import os
import sys
import subprocess
import threading
from tkinter import Tk, Label, Button, filedialog

# Import functions
from csv_clear import limpieza_csv
from array_power import process_array
from training_data import training
from plottings import plotting

def update_message(label, text):
    label.config(text=text)
    label.update_idletasks()

def open_output_folder(folder_path):
    # Open the output folder in the file explorer
    if sys.platform == "win32":
        os.startfile(folder_path)
    elif sys.platform == "darwin":
        subprocess.Popen(["open", folder_path])
    else:
        subprocess.Popen(["xdg-open", folder_path])

def start_processing(file_labels, status_label, start_button, output_button):
    def process():
        try:
            # Initialize Tkinter
            root = Tk()
            root.withdraw()  # Hide the main Tkinter window

            # Request the initial CSV file
            initial_file = filedialog.askopenfilename(title="Seleccione el archivo de datos")
            if not initial_file:
                update_message(file_labels['initial_file'], "No se seleccionó ningún archivo de datos.")
                return

            update_message(file_labels['initial_file'], "Archivo de datos seleccionado: " + initial_file)

            # Request the Meteo file
            extra_file = filedialog.askopenfilename(title="Seleccione el archivo de meteo")
            if not extra_file:
                update_message(file_labels['extra_file'], "No se seleccionó ningún archivo de meteo.")
                return

            update_message(file_labels['extra_file'], "Archivo de meteo seleccionado: " + extra_file)

            # Request the output folder
            output_folder = filedialog.askdirectory(title="Seleccione la carpeta de destino para los archivos generados")
            if not output_folder:
                update_message(file_labels['output_folder'], "No se seleccionó ninguna carpeta de destino.")
                return

            update_message(file_labels['output_folder'], "Carpeta de destino seleccionada: " + output_folder)

            # Base name and extension of the initial file
            initial_filename = os.path.basename(initial_file)
            base_name, ext = os.path.splitext(initial_filename)

            # Define names of processed files in the selected destination folder
            arch1 = os.path.join(output_folder, f"{base_name}_csv_clear{ext}")
            arch2 = os.path.join(output_folder, f"{base_name}_array_power{ext}")
            arch3 = os.path.join(output_folder, f"{base_name}_training_data{ext}")
            arch4 = os.path.join(output_folder, f"{base_name}_plottings{ext}")

            # Data processing
            update_message(status_label, "Procesando los datos...")
            limpieza_csv(initial_file, arch1)
            update_message(status_label, "Limpieza de datos completada. Procesando datos ...")

            process_array(arch1, arch2)
            update_message(status_label, "Procesamiento del array completado. Obteniendo resultados ...")

            training(arch2, extra_file, arch3)

            plotting(arch3, arch4, output_folder)

            update_message(status_label, "Procesado finalizado")
            output_button.config(command=lambda: open_output_folder(output_folder), state="normal")
            output_button.pack()

        except Exception as e:
            update_message(status_label, f"Error durante el procesamiento: {e}")
            start_button.config(state="normal")

    # Execute the processing in a separate thread
    threading.Thread(target=process).start()

def on_closing(root):
    root.destroy()
    sys.exit()

def main():
    # Create the main Tkinter window
    root = Tk()
    root.title("Cálculo de ganancia energética y obtención del punto de máxima potencia")

    # Close function to end the program
    root.protocol("WM_DELETE_WINDOW", lambda: on_closing(root))

    # Screen dimensions
    screen_width = root.winfo_screenwidth()
    screen_height = root.winfo_screenheight()

    # Set the window dimensions (75% of the screen)
    window_width = int(screen_width * 0.75)
    window_height = int(screen_height * 0.75)

    # Center the window
    position_right = int((screen_width - window_width) / 2)
    position_down = int((screen_height - window_height) / 2)
    root.geometry(f"{window_width}x{window_height}+{position_right}+{position_down}")

    # Configure the title label
    title_label = Label(root, text="Cálculo de ganancia energética y obtención del punto de máxima potencia", font=("Helvetica", 16))
    title_label.pack(pady=20)

    # Start processing button
    start_button = Button(root, text="Iniciar procesado", font=("Helvetica", 14))
    start_button.pack(pady=50)

    # Labels for selected files and output folder
    initial_file_label = Label(root, text="", font=("Helvetica", 12))
    initial_file_label.pack(pady=5)
    extra_file_label = Label(root, text="", font=("Helvetica", 12))
    extra_file_label.pack(pady=5)
    output_folder_label = Label(root, text="", font=("Helvetica", 12))
    output_folder_label.pack(pady=5)

    # Status message label
    status_message_label = Label(root, text="", font=("Helvetica", 12))
    status_message_label.pack(pady=20)

    # Button to open the output folder
    output_button = Button(root, text="Abrir carpeta de salida", state="disabled", font=("Helvetica", 12))

    # Start button comand
    start_button.config(command=lambda: start_processing(
        {'initial_file': initial_file_label, 'extra_file': extra_file_label, 'output_folder': output_folder_label},
        status_message_label,
        start_button,
        output_button
    ))
    root.mainloop()

if __name__ == "__main__":
    main()
