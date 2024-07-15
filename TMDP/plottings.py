# -*- coding: utf-8 -*-
"""
Created on Wed Jun  7 18:02:50 2023

@author: Jesus

Modified by Luis Fco.
"""
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
import pvmismatch as pvm
import sys
import os
from datetime import datetime

def save_plot(fig, ax, title, folder, date):
    filename = f"{title}_{date.strftime('%Y-%m-%d')}.png"
    filepath = os.path.join(folder, filename)
    fig.savefig(filepath)
    plt.close(fig)

def plotting(input_file, output_file, output_folder):

    bifaciality = 0.75
    # Module, system and cell initialization
    module = pvm.PVmodule(cell_pos=pvm.pvmismatch_lib.pvmodule.STD72)

    # For 2V
    system = pvm.PVsystem(numberMods=1, numberStrs=2, pvmods=module)

    # Normalized power
    system.setSuns(1)
    system.setTemps(298.15)
    normalized_power = system.Pmp

    # Import dataframes
    data = input_file

    # First, datalogger data
    processed_df = pd.read_csv(data,
                               sep=",",  # two types of separation
                               header=0,  # first row used as index
                               skiprows=0,  # skip index
                               engine='python',
                               parse_dates=True,
                               index_col=0)

    processed_df.index = pd.to_datetime(processed_df.index) 

    # Get the date from the dataframe
    date = processed_df.index[0]
    
    # Create a subfolder for images
    figure_folder = os.path.join(output_folder, 'Figures')
    if not os.path.exists(figure_folder):
        os.makedirs(figure_folder)

    # Power and ghi
    fig, ax1 = plt.subplots(figsize=(10, 6))
    cmap = mpl.colormaps['inferno']
    scatter1 = ax1.scatter(processed_df['GHI'], processed_df['optimal power'], c=processed_df['clearness index'],
                           cmap=cmap, label='Pmax at optimum tilt')
    ax1.set_xlabel('Global Horizontal Irradiance [Suns]')
    ax1.set_ylabel('Referenced at STC')
    ax1.set_title('Optimal power')
    ax1.legend(loc='best')
    ax1.tick_params(axis='x', rotation=45)
    cbar = fig.colorbar(scatter1, ax=ax1)
    cbar.ax.set_ylabel('Clearness index')
    plt.grid()
    plt.tight_layout()  # Adjust spacing between subplots
    save_plot(fig, ax1, 'Optimal_power', figure_folder, date)

    # Bifacial Gains
    cmap = mpl.colormaps['viridis']
    fig, ax5 = plt.subplots(figsize=(10, 6))
    scatter5 = ax5.scatter(processed_df.index, processed_df['Optimal Bifacial gains'], c=processed_df['clearness index'],
                           cmap=cmap, label='Bifacial Gains (energy)')
    ax5.set_xlabel('DateTime')
    ax5.set_ylabel('[%]')
    ax5.set_title('Bifacial Gains')
    ax5.legend(loc='best')
    ax5.tick_params(axis='x', rotation=45)
    cbar = fig.colorbar(scatter5, ax=ax5)
    cbar.ax.set_ylabel('Clearness index')
    plt.grid()
    plt.tight_layout()  # Adjust spacing between subplots
    save_plot(fig, ax5, 'Bifacial_Gains', figure_folder, date)

    # Angle and tilt
    fig, ax2 = plt.subplots(figsize=(10, 6))
    ax2.plot(processed_df.index, processed_df['tilt'], color='orange', label='Ephemeris tilt', linewidth=3)
    scatter2 = ax2.scatter(processed_df.index, processed_df['optimal angle'], color='blue', label='Optimal tilt')
    ax2.set_xlabel('Datetime')
    ax2.set_ylabel('Degrees [$º$]')
    ax2.set_ylim([-60, 60])
    ax2.set_title('Optimum tilt vs Ephemeris')
    ax2.legend(loc='best')
    ax2.tick_params(axis='x', rotation=45)
    ax2.legend(loc='best')
    plt.grid()
    plt.tight_layout()  # Adjust spacing between subplots
    save_plot(fig, ax2, 'Optimum_tilt_vs_Ephemeris', figure_folder, date)

    # Energy gain
    cmap = mpl.colormaps['plasma']
    fig, ax3 = plt.subplots(figsize=(10, 6))
    scatter3 = ax3.scatter(processed_df['clearness index'], 100 * (processed_df['optimal power'] - processed_df['ephemeris power']) / processed_df['ephemeris power'], c=processed_df['GHI'],
                           cmap=cmap, label='Energy gain')
    ax3.set_xlabel('Clearness index')
    ax3.set_ylabel('Energy gain [%]')
    ax3.set_title('Energy gain')
    ax3.legend(loc='best')
    ax3.tick_params(axis='x', rotation=45)
    cbar = fig.colorbar(scatter3, ax=ax3)
    cbar.ax.set_ylabel('GHI [Suns]')
    plt.grid()
    plt.tight_layout()  # Adjust spacing between subplots
    save_plot(fig, ax3, 'Energy_gain', figure_folder, date)

    # GHI and Power
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(processed_df.index, processed_df['GHI'], color='green', label='GHI', linewidth=2)
    ax.scatter(processed_df.index, processed_df['optimal power'] * normalized_power, color='blue', label='Optimal Power')
    ax.set_xlabel('Datetime')
    ax.set_ylabel('GHI [W/m2] / Optimal Power [W]')
    ax.set_title('GHI and Optimal Power over Time')
    ax.legend(loc='best')
    ax.tick_params(axis='x', rotation=45)
    plt.grid()
    plt.tight_layout()  # Adjust spacing between subplots
    save_plot(fig, ax, 'GHI_and_Optimal_Power', figure_folder, date)

    # Angular difference vs clarity index
    cmap = mpl.colormaps['magma']
    fig, ax4 = plt.subplots(figsize=(10, 6))
    scatter4 = ax4.scatter(processed_df['clearness index'], abs(processed_df['optimal angle'] - processed_df['tilt']), c=processed_df['GHI'],
                           cmap=cmap, label='Optimal difference')
    ax4.set_xlabel('Clearness index')
    ax4.set_ylabel('Degrees [º]')
    ax4.set_title('Optimal tilt vs clearness index')
    ax4.legend(loc='best')
    ax4.tick_params(axis='x', rotation=45)
    ax4.set_ylim([-2, 40])
    cbar = fig.colorbar(scatter4, ax=ax4)
    cbar.ax.set_ylabel('GHI [Suns]')
    plt.grid()
    plt.tight_layout()  # Adjust spacing between subplots
    save_plot(fig, ax4, 'Optimal_tilt_vs_clearness_index', figure_folder, date)

    optimal_power = processed_df['optimal power'] * normalized_power / 60  # kWh
    optimal_power_non_bifacial = processed_df['optimal power non bifacial'] * normalized_power / 60  # kWh
    optimal_energy = optimal_power.sum()
    optimal_energy_non_bifacial = optimal_power_non_bifacial.sum()
    bifacial_gains = 100 * ((optimal_energy - optimal_energy_non_bifacial) / optimal_energy_non_bifacial)
    bifacial_gains_irr = 100 * bifaciality * ((processed_df['BE'].sum() + processed_df['BW'].sum()) / (processed_df['FE'].sum() + processed_df['FW'].sum()))

    ephemeris_power = processed_df['ephemeris power'] * normalized_power / 60  # kWh
    ephemeris_energy = ephemeris_power.sum()
    energy_gain = 100 * (optimal_energy - ephemeris_energy) / (ephemeris_energy)

    global_results = {'Optimal energy [kWh]': round(optimal_energy, 2),
                      'Ephemeris energy [kWh]': round(ephemeris_energy, 2),
                      'Bifacial Gains [%]': round(bifacial_gains, 2),
                      'Irradiance Bifacial Gains [%]': round(bifacial_gains_irr, 2),
                      'Energy Gain [%]': round(energy_gain, 2)}

    # Save csv
    pd.DataFrame([global_results]).to_csv(output_file, index=False)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Uso: python plottings.py <input_file> <output_file> <output_folder>")
        sys.exit(1)
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    output_folder = sys.argv[3]
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    plotting(input_file, output_file, output_folder)
