# -*- coding: utf-8 -*-
"""
Program which calculates real power from solar bifacial system, based on experimental
front and back irradiance data

@author: Luis
"""
import sys, os, time
sys.path.append(os.path.dirname(__file__))
import datalogger as dl
from datalogger import data_import
import numpy as np
import pandas as pd
import pvmismatch as pvm
import matplotlib.pyplot as plt
import multiprocessing
from datetime import timedelta
from scipy.interpolate import UnivariateSpline

# Sensor distribution
BE = ['CH1', 'CH2', 'CH3', 'CH4']
FE = ['CH5', 'CH6', 'CH7', 'CH8']
BW = ['CH9', 'CH10', 'CH11', 'CH12']
FW = ['CH13', 'CH14', 'CH15', 'CH16']
sys = BE + FE + BW + FW
bifaciality = 0.75

# Module, system and cell initialization

module = pvm.PVmodule(cell_pos = pvm.pvmismatch_lib.pvmodule.STD72)

# For 1V
# system = pvm.PVsystem(numberMods=2,numberStrs=1, pvmods = module)

# For 2V
system = pvm.PVsystem(numberMods=1,numberStrs=2, pvmods = module)

#Normalized power
system.setSuns(1)
system.setTemps(298.15)
normalized_power = system.Pmp

# Number of rows and columns in a module
n = 0
m = 0

for i in module.subStrCells:
    n += i
    
m = module.numberCells
m /= n
m = int(m)

# Initialize variables
power = None
REPROCESS_ARRAY = True


#Correción angular empírica

#Array de error este - oeste
error_EW = np.array([[-50. , -52.1 ],
                   [-45. , -47],
                   [-40. , -41.5],
                   [-35. , -36.9],
                   [-30. , -32],
                   [-25. , -26.7],
                   [-20. , -21.7],
                   [-15. , -16.8],
                   [-10. , -11.7],
                   [ -5. ,  -6.7],
                   [  0. ,   -1.6],
                   [  5. ,   3.3 ],
                   [ 10. ,  8.3],
                   [ 15. ,  13.5],
                   [ 20. ,  18.4],
                   [ 25. ,  23.8],
                   [ 30. ,  30],
                   [ 35. ,  35.1],
                   [ 40. ,  40.4],
                   [ 45. ,  45.4],
                   [ 50. ,  50.3]])

#Separación de las columnas
error_EW_x = error_EW[:, 0]
error_EW_y = error_EW[:, 1]

#Spline de los ángulos
error_EW_spline = UnivariateSpline(error_EW_x, error_EW_y, s=0)

#Array de error oeste - este
error_WE = np.array([[ 50. , 50.5 ],
                   [ 45. , 45.6],
                   [ 40. , 40.7],
                   [ 35. , 35.6 ],
                   [ 30. , 30.9],
                   [ 25. , 25.9],
                   [ 20. , 20.7],
                   [ 15. , 15.8],
                   [ 10. , 10.8],
                   [ 5. ,  5.8],
                   [  0. ,   0.7],
                   [-5. ,   -4.7 ],
                   [-10. ,  -10.9],
                   [-15. ,  -16.1],
                   [-20. ,  -21.1],
                   [-25. ,  -26.1],
                   [-30. ,  -31],
                   [-35. ,  -36.3],
                   [-40. ,  -41.1],
                   [-45. ,  -46.1],
                   [-50. ,  -51.3]])

#Invierto el array para poder crear el spline
error_WE = np.flipud(error_WE)

#Separación de las columnas
error_WE_x = error_WE[:, 0]
error_WE_y = error_WE[:, 1]

#Spline de los ángulos
error_WE_spline = UnivariateSpline(error_WE_x, error_WE_y, s=0)

# Convertir a un DataFrame
#df_error_motor = pd.DataFrame(error_motor, columns=['Nominal', 'Real'])


# Process function
def process_array(input_file, output_file):
    
    # Global variables
    global plate_west, plate_east, plate_west_non_bif, plate_east_non_bif
    global results, results_non_bif, suns
    global power, training_data
    global filtered_data
    global system

    # Import data from the datalogger
    data = dl.data_import(input_file,'datalogger')
    
    #Merge miliseconds
    data.index = data.index + pd.to_timedelta(data['ms'], unit='ms')
    
    # Convert data to irradiance, temperature correction, and smoothing
    filtered_data = dl.datalogger_filter(df = data,
                                      mean_coeff = 1, 
                                      irr_coef = dl.irr_coef, 
                                      ch_temp = 'CH19')
    
    # Logic sweep channels
    #ATENCION CAMBIADO
    # data['aux'] = data['CH17']
    # data['CH17'] = data['CH18']
    # data['CH18'] = data['aux']
    # data.drop(columns=['aux'], inplace=True)
    
    filtered_data['CH17'] = data['CH17']
    filtered_data['CH18'] = data['CH18']

    #GHI
    filtered_data['GHI'] = data['CH20'] * (1000/76.63)
    
    # Delete data without info on ch17 or ch18
    threshold = 1
    filtered_data = filtered_data[(filtered_data['CH17'] > threshold) ^ (filtered_data['CH18'] > threshold)]
    
    # Create angle column
    filtered_data['angle'] = np.nan
    
    # Give values to angle column
    flag_ch17 = False
    flag_ch18 = False
    
    index_array = []
    
    for i in filtered_data.index:
        if (filtered_data['CH17'].loc[i] > threshold).any() and (filtered_data['CH18'].loc[i] < threshold).any():
            flag_ch17 = True
            index_array.append(i)
            
        elif (filtered_data['CH18'].loc[i] > threshold).any() and (filtered_data['CH17'].loc[i] < threshold).any():
            flag_ch18 = True
            index_array.append(i)
            
        if (filtered_data['CH17'].loc[i] < threshold).any() and flag_ch17:
            flag_ch17 = False

            # Create linspace of angles
            angles = np.linspace(-50, 50, len(index_array))
            real_angles = error_EW_spline(angles)
            for j, index in enumerate(index_array):
                filtered_data.at[index, 'angle'] = real_angles[j]
                
            #If there was an error
            if len(index_array) < 8 or len(index_array) > 200:
                filtered_data.at[index, 'angle'] = 180
                
            # Reset array of indexes
            index_array = []
            
        elif (filtered_data['CH18'].loc[i] < threshold).any() and flag_ch18:
            flag_ch18 = False
            
            # Create linspace of angles
            angles = np.linspace(-50, 50, len(index_array))
            real_angles = error_WE_spline(angles)
            real_angles = np.flipud(real_angles)
            for j, index in enumerate(index_array):
                filtered_data.at[index, 'angle'] = real_angles[j]
                
            #If there was an error
            if len(index_array) < 8 or len(index_array) > 200:
                filtered_data.at[index, 'angle'] = 180
                
            # Reset array of indexes
            index_array = []
            
    filtered_data = filtered_data[filtered_data['angle']<90]

    # Create all plate irradiance levels through interpolation
    plate_west, plate_east, plate_west_non_bif, plate_east_non_bif = create_plates_df(filtered_data)
    plate_west = plate_west.interpolate(method = 'linear', axis = 1)
    plate_east = plate_east.interpolate(method = 'linear', axis = 1)
    plate_west_non_bif = plate_west_non_bif.interpolate(method = 'linear', axis = 1)
    plate_east_non_bif = plate_east_non_bif.interpolate(method = 'linear', axis = 1)
    
    #To measure performance ratio of the system
    start_time = time.time()
    
    if REPROCESS_ARRAY:
        
        # Multiprocess loop
        with multiprocessing.Pool() as pool:
            
            # Arguments of the calc_power function
            args = [(x, plate_west, plate_east, system, m, n) for x in filtered_data.index]
            args_non_bif = [(x, plate_west_non_bif, plate_east_non_bif, system, m, n) for x in filtered_data.index]
            
            # Actual calculation of system power
            results = pool.starmap(calc_power, args)
            results_non_bif = pool.starmap(calc_power, args_non_bif)
        
        # Rearrange results into a dataframe
        results = pd.DataFrame(results, columns = ['DateTime', 'power_value'])
        results_non_bif = pd.DataFrame(results_non_bif, columns = ['DateTime', 'power_value'])
        power = pd.merge(results, results_non_bif, on='DateTime')
        power.set_index('DateTime', inplace=True)
        power.columns = ['power_value', 'power_value_non_bif']
        power /= normalized_power
        power.index = results['DateTime']
        
        # Save CSV file with the data obtained
    else:
        power = np.loadtxt('power_array.txt')
    
    # Finish time performance measure
    end_time = time.time()
    processing_time = end_time - start_time
    print(f"Processing time: {processing_time:.4f} seconds")

    # Plot data
    plt.figure(figsize = (10,6))
    power['power_value'].plot()
    plt.title('Power')
    plt.tight_layout()
    
    # Add angles
    power['angle'] = filtered_data['angle']
    power['GHI'] = filtered_data['GHI']
    
    # Irradiance channels
    for i in range (1, 17):
        power['CH' + str(i)] = filtered_data['W' + str(i)]
        power['CH' + str(i)] = power['CH' + str(i)].clip(lower=0.01)
    
    # Mean irradiance
    power['BE'] = power[BE].mean(axis = 1)
    power['FE'] = power[FE].mean(axis = 1)
    power['BW'] = power[BW].mean(axis = 1)
    power['FW'] = power[FW].mean(axis = 1)
    
    # Mismatch loss factor of the plates
    power['Mismatch BE'] = power[BE].apply(lambda x: 100*(x.max() - x.min()) / x.mean(), axis=1)
    power['Mismatch FE'] = power[FE].apply(lambda x: 100*(x.max() - x.min()) / x.mean(), axis=1)
    power['Mismatch BW'] = power[BW].apply(lambda x: 100*(x.max() - x.min()) / x.mean(), axis=1)
    power['Mismatch FW'] = power[FW].apply(lambda x: 100*(x.max() - x.min()) / x.mean(), axis=1)       
    
    # Save file
    my_cols = ['power_value', 'power_value_non_bif', 'angle', 'GHI', 
               'BE-exterior', 'BE-mid-exterior', 'BE-mid-interior', 'BE-interior',
               'FE-exterior', 'FE-mid-exterior', 'FE-mid-interior', 'FE-interior',
               'BW-exterior', 'BW-mid-exterior', 'BW-mid-interior', 'BW-interior',
               'FW-exterior', 'FW-mid-exterior', 'FW-mid-interior', 'FW-interior',
               'BE', 'FE', 'BW', 'FW',
               'BE mismatch', 'FE mismatch', 'BW mismatch', 'FW mismatch']

    power = power.rename(columns=dict(zip(power.columns, my_cols)))
    power.to_csv(output_file, index = True)
    
# Place sensor data into a dataframe of all levels of irradiance
def create_plates_df(filtered_data):
    plate_west = pd.DataFrame(columns = range(0,m))
    plate_east = pd.DataFrame(columns = range(0,m))
    plate_west_non_bif = pd.DataFrame(columns = range(0,m))
    plate_east_non_bif = pd.DataFrame(columns = range(0,m))
    
    for i in range (0, m):
        
        # Exterior sensor
        if i == 0 or i == 1:
            plate_east[i] = bifaciality * filtered_data['W1'] + filtered_data['W5']
            plate_west[i] = bifaciality * filtered_data['W9'] + filtered_data['W13']
            plate_east_non_bif[i] = filtered_data['W5']
            plate_west_non_bif[i] = filtered_data['W13']
            
        # Mid exterior sensor
        elif i == (m / 2) - 1:
            plate_east[i] = bifaciality * filtered_data['W2'] + filtered_data['W6']
            plate_west[i] = bifaciality * filtered_data['W10'] + filtered_data['W14']
            plate_east_non_bif[i] = filtered_data['W6']
            plate_west_non_bif[i] = filtered_data['W14']
            
            
        # Mid interior sensor
        elif i == (m / 2):
            plate_east[i] = bifaciality * filtered_data['W3'] + filtered_data['W7']
            plate_west[i] = bifaciality * filtered_data['W11'] + filtered_data['W15']
            plate_east_non_bif[i] = filtered_data['W7']
            plate_west_non_bif[i] = filtered_data['W15']
            
        # Interior sensor
        elif i == m - 1 or i == m - 2:
            plate_east[i] = bifaciality * filtered_data['W4'] + filtered_data['W8']
            plate_west[i] = bifaciality * filtered_data['W12'] + filtered_data['W16']
            plate_east_non_bif[i] = filtered_data['W8']
            plate_west_non_bif[i] = filtered_data['W16']
            
        # Spots without sensors (they will be interpolated)
        else:
            plate_east[i] = np.empty(len(filtered_data)) * np.nan
            plate_west[i] = np.empty(len(filtered_data)) * np.nan
            plate_east_non_bif[i] = np.empty(len(filtered_data)) * np.nan
            plate_west_non_bif[i] = np.empty(len(filtered_data)) * np.nan
            
    # Set temperature data
    plate_west['T'] = filtered_data['CH19']
    plate_east['T'] = filtered_data['CH19']
    plate_west_non_bif['T'] = filtered_data['CH19']
    plate_east_non_bif['T'] = filtered_data['CH19']
        
    return plate_west, plate_east, plate_west_non_bif, plate_east_non_bif

# Function that outputs peak power with given irradiance maps of the system
def calc_power(x, plate_west, plate_east, system, m, n):

    # Arrays of irradiance of all cells
    irr_west = np.zeros(module.numberCells)
    irr_east = np.zeros(module.numberCells)
    
    # Data from the datalogger
    data_west = plate_west.loc[x]
    data_east = plate_east.loc[x]
    
    # Create irradiance arrays
    for i in range(0, module.numberCells):
        irr_west[i] = data_west[i // m]
    
    # Place irradiance in the given order of the solar pannel module 
    # variable i represents diodes
    for i in module.cell_pos:
        # variable j represents columns
        for j in i:
            
            aux = 0
            # variable k represents dictionary of cells
            for k in j:
                idx = k['idx']
                irr_west[idx] = data_west[aux]/1000
                irr_east[idx] = data_east[aux]/1000
                aux += 1
    
    # There can't be non zero irradiance values (the funct won't work)
    irr_west[irr_west <= 0] = 0.001
    irr_east[irr_east <= 0] = 0.001

    # For two modules in parallel
    dict_suns = {0: {0: irr_west},
                  1: {0: irr_east}}
    
    # For two modules in series
    # dict_suns = {0: {0: irr_west, 1: irr_east}}
    
    # Set irradiance 
    system.setSuns(dict_suns)
    
    # Set temperature
    system.setTemps(data_west['T'] + 273.15)
    
    # Get peak power
    power = system.Pmp
    
    # Return data
    return x, power
    
# Function which calculates IV curve of specific points in the data
def iv_irr_data(hour):
    
    global plates
    # Plot the IV curve
    x, power = calc_power(hour, plate_west, plate_east, system, m, n)
    
    
    fig1 = plt.figure(figsize = (10, 6))
    system.plotSys(sysPlot = fig1)
    fig1.tight_layout()
    
    fig2 = plt.figure(figsize = (10, 6))
    plates = filtered_data[sys].loc[hour]
    plt.bar(plates.index, plates[sys])
    fig2.tight_layout()
    print(f'{hour}\n Pmp = {system.Pmp} W. \n Voc = {system.Voc} V. \n Isc = {system.Isc} A.')

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Uso: python Prueba_array.py <input_file> <output_file> <file_type>")
        sys.exit(1)
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    process_array(input_file, output_file)
