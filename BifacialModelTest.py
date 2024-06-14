# -*- coding: utf-8 -*-
"""
Created on Thu Jun 13 11:00:57 2024

@author: Luis Francisco
"""

import pandas as pd
from pvlib import location, tracking, irradiance
import matplotlib.pyplot as plt
from sklearn.metrics import mean_squared_error, r2_score

import datalogger_no_merged as dl

# Suprime las advertencias innecesarias
import warnings
warnings.filterwarnings(action='ignore', module='pvfactors')

# Definir la función get_irradiance
def get_irradiance(site_location, date, tilt, surface_azimuth):
    # Crea intervalos de 10 minutos para un día completo
    times = pd.date_range(date, freq='10min', periods=6*24, tz=site_location.tz)
    # Genera datos de cielo despejado usando el modelo Ineichen
    clearsky = site_location.get_clearsky(times)
    # Obtiene la posición solar
    solar_position = site_location.get_solarposition(times=times)
    # Transpone GHI a POA
    POA_irradiance = irradiance.get_total_irradiance(
        surface_tilt=tilt,
        surface_azimuth=surface_azimuth,
        dni=clearsky['dni'],
        ghi=clearsky['ghi'],
        dhi=clearsky['dhi'],
        solar_zenith=solar_position['apparent_zenith'],
        solar_azimuth=solar_position['azimuth'])
    # Retorna un DataFrame con GHI y POA
    return pd.DataFrame({'GHI': clearsky['ghi'], 'POA': POA_irradiance['poa_global']})

# Cargar los datos de GHI medidos
data = dl.data_import('datalogger')

# Merge miliseconds
data.index = data.index + pd.to_timedelta(data['ms'], unit='ms')

# Convert data to irradiance, temperature correction, and smoothing
filtered_data = dl.datalogger_filter(df=data,
                                     mean_coeff=1,
                                     irr_coef=dl.irr_coef,
                                     ch_temp='CH19')

filtered_data['GHI'] = data['CH20'] * (1000/76.63)

filtered_data.index = filtered_data.index.tz_localize('Europe/Madrid')

# Definir la ubicación y el rango de tiempo
lat, lon = 40.453201, -3.726968  # Coordenadas de Madrid
tz = 'Europe/Madrid'
site_location = location.Location(lat, lon, tz=tz, name='Madrid')

# Usar la fecha de los datos filtrados para obtener el irradiance modelado
date = filtered_data.index[0].date()

# Obtener la irradiancia GHI y POA usando la función definida
irradiance_data = get_irradiance(site_location, date, tilt=0, surface_azimuth=180)

# Crear un DataFrame para el día completo con los datos modelados
full_day_df = irradiance_data

# Calcular la orientación del rastreador HSAT para el día completo
gcr = 0.3  # Ratio de cobertura del suelo
max_phi = 50  # Ángulo máximo del rastreador
solar_position = site_location.get_solarposition(full_day_df.index)
orientation = tracking.singleaxis(solar_position['apparent_zenith'],
                                  solar_position['azimuth'],
                                  max_angle=max_phi,
                                  backtrack=True,
                                  gcr=gcr)

# Calcular la POA utilizando el modelo de rastreador HSAT
poa_tracker = irradiance.get_total_irradiance(
    surface_tilt=orientation['surface_tilt'],
    surface_azimuth=orientation['surface_azimuth'],
    dni=irradiance_data['GHI'],
    ghi=irradiance_data['GHI'],
    dhi=irradiance_data['GHI'],
    solar_zenith=solar_position['apparent_zenith'],
    solar_azimuth=solar_position['azimuth']
)

full_day_df['POA_Tracker'] = poa_tracker['poa_global']

# Añadir los datos medidos al DataFrame del día completo
full_day_df = full_day_df.join(filtered_data['GHI'], how='left', rsuffix='_Measured')

# Visualizar los datos para comparar
full_day_df[['GHI_Measured', 'POA_Tracker']].plot(title='Comparación de GHI Medido y POA Modelado (HSAT) para un Día Completo')
plt.ylabel('Irradiancia (W/m^2)')
plt.xlabel('Hora')
plt.show()

# Calcular métricas de comparación solo para el periodo de datos medidos
measured_period = full_day_df.dropna(subset=['GHI_Measured'])

# Calcular RMSE
rmse = mean_squared_error(measured_period['GHI_Measured'], measured_period['POA_Tracker'], squared=False)
print(f'RMSE: {rmse:.2f} W/m^2')

# Calcular R²
r2 = r2_score(measured_period['GHI_Measured'], measured_period['POA_Tracker'])
print(f'R²: {r2:.2f}')
