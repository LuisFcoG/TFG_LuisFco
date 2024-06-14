# -*- coding: utf-8 -*-
"""
Created on Wed Jun 12 17:16:24 2024

@author: Luis Francisco
"""

import sys, os, time
sys.path.append(os.path.dirname(__file__))
import datalogger_no_merged as dl
import numpy as np
import pandas as pd
import pvmismatch as pvm
import matplotlib.pyplot as plt
import multiprocessing
from tkinter import filedialog
from datetime import timedelta
from scipy.interpolate import UnivariateSpline

import matplotlib.pyplot as plt
import matplotlib as mpl

def main():
    
    # Import data from the datalogger
    data = dl.data_import('datalogger')
    
    #Merge miliseconds
    data.index = data.index + pd.to_timedelta(data['ms'], unit='ms')
    
    # Convert data to irradiance, temperature correction, and smoothing
    filtered_data = dl.datalogger_filter(df = data,
                                      mean_coeff = 1, 
                                      irr_coef = dl.irr_coef, 
                                      ch_temp = 'CH19')
    
    filtered_data['CH17'] = data['CH17']
    filtered_data['CH18'] = data['CH18']

    #GHI
    filtered_data['GHI'] = data['CH20'] * (1000/76.63)
    
    # Generate plots for each channel from CH1 to CH16 and GHI
    channels = [f'CH{i}' for i in range(1, 17)] + ['GHI']

    for channel in channels:
        plt.figure(figsize=(10, 6))
        plt.plot(filtered_data.index, filtered_data[channel], label=channel)
        plt.xlabel('Tiempo')
        plt.ylabel('Voltaje')
        plt.title(f'{channel} vs Tiempo')
        plt.legend()
        plt.grid(True)
        plt.show()
        
    # Generate plot with dual Y-axes
    
    fig, ax1 = plt.subplots(figsize=(14, 8))
    
    # Plot channels CH1 to CH16 on the first Y-axis
    channels = [f'CH{i}' for i in range(1, 17)]
    for channel in channels:
        ax1.plot(filtered_data.index, filtered_data[channel], label=channel)

    ax1.set_xlabel('Time')
    ax1.set_ylabel('Voltage (mV)')
    ax1.tick_params(axis='y')
    ax1.legend(loc='upper left')
    ax1.grid(True)

    # Create a second Y-axis for GHI
    ax2 = ax1.twinx()
    ax2.plot(filtered_data.index, filtered_data['GHI'], 'k-', label='GHI', linewidth=2)
    ax2.set_ylabel('GHI (W/m²)')
    ax2.tick_params(axis='y')

    # Add legend for GHI
    fig.tight_layout()
    fig.legend(loc='upper right')

    plt.title('Channels (CH1 to CH16) and GHI vs Time')
    plt.show()

    
if __name__ == "__main__":
    main()