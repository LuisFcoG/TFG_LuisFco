# -*- coding: utf-8 -*-
"""
Created on Thu Jun 13 12:46:02 2024

@author: Luis Francisco
"""

import pandas as pd
import numpy as np
from pvlib import location
from pvlib import tracking
from pvlib import irradiance
from pvlib.bifacial.pvfactors import pvfactors_timeseries
from pvlib import temperature
from pvlib import pvsystem
import matplotlib.pyplot as plt
import warnings
import datalogger_no_merged as dl

# Suprimir las advertencias innecesarias
import warnings
warnings.filterwarnings(action='ignore', module='pvfactors')

# Definir la ubicación y el rango de tiempo para Madrid el 11 de junio
lat, lon = 40.453201, -3.726968  # Coordenadas de Madrid
tz = 'Europe/Madrid'

# Cargar los datos de GHI medidos
data = dl.data_import('datalogger')

# Merge miliseconds
data.index = data.index + pd.to_timedelta(data['ms'], unit='ms')

# Convertir datos a irradiancia, corrección de temperatura y suavizado
filtered_data = dl.datalogger_filter(df=data,
                                     mean_coeff=1,
                                     irr_coef=dl.irr_coef,
                                     ch_temp='CH19')

filtered_data['GHI'] = data['CH20'] * (1000/76.63)
filtered_data.index = filtered_data.index.tz_localize('Europe/Madrid')

# Crear objeto de ubicación
site_location = location.Location(lat, lon, tz=tz, name='Madrid')

# Obtener datos de posición solar
solar_position = site_location.get_solarposition(filtered_data.index)

# Convertir GHI a DNI usando el modelo DISC
dni_dhi = irradiance.disc(filtered_data['GHI'], solar_position['apparent_zenith'], filtered_data.index)

# Calcular DHI
dni_dhi['dhi'] = filtered_data['GHI'] - dni_dhi['dni'] * np.cos(np.radians(solar_position['apparent_zenith']))

# Añadir DNI y DHI a filtered_data
filtered_data['DNI'] = dni_dhi['dni']
filtered_data['DHI'] = dni_dhi['dhi']

# Calcular la orientación del tracker
gcr = 0.3
max_phi = 50
orientation = tracking.singleaxis(solar_position['apparent_zenith'],
                                  solar_position['azimuth'],
                                  max_angle=max_phi,
                                  backtrack=True,
                                  gcr=gcr)

# Simular explícitamente en pvarray con 3 filas, con el sensor colocado en la fila del medio
irrad = pvfactors_timeseries(
    solar_azimuth=solar_position['azimuth'],
    solar_zenith=solar_position['apparent_zenith'],
    surface_azimuth=orientation['surface_azimuth'],
    surface_tilt=orientation['surface_tilt'],
    axis_azimuth=180,
    timestamps=filtered_data.index,
    dni=filtered_data['DNI'],
    dhi=filtered_data['DHI'],
    gcr=gcr,
    pvrow_height=3,
    pvrow_width=4,
    albedo=0.2,
    n_pvrows=3,
    index_observed_pvrow=1
)


# Convertir a DataFrame de pandas
irrad = pd.concat(irrad, axis=1)

# Usar el factor de bifacialidad y los resultados de pvfactors para crear la irradiancia efectiva
bifaciality = 0.75
effective_irrad_bifi = irrad['total_abs_front'] + (irrad['total_abs_back'] * bifaciality)

# Obtener la temperatura de la célula usando el modelo Faiman
temp_cell = temperature.faiman(effective_irrad_bifi, temp_air=25, wind_speed=1)

# Usar el modelo pvwatts_dc y los parámetros detallados anteriormente,
# establecer pdc0 y devolver la potencia DC tanto para bifacial como monofacial
pdc0 = 1
gamma_pdc = -0.0043
pdc_bifi = pvsystem.pvwatts_dc(effective_irrad_bifi,
                               temp_cell,
                               pdc0,
                               gamma_pdc=gamma_pdc).fillna(0)

# Crear la figura y el eje
fig, ax1 = plt.subplots(figsize=(10, 6))

# Representar la Potencia DC Bifacial en el primer eje Y
ax1.plot(pdc_bifi.index, pdc_bifi, label='Potencia DC Bifacial', color='tab:blue')
ax1.set_xlabel('Hora')
ax1.set_ylabel('Potencia DC', color='tab:blue')
ax1.tick_params(axis='y', labelcolor='tab:blue')
ax1.set_ylim(0, pdc_bifi.max()*1.1)  # Ajustar los límites del eje Y para la Potencia DC

# Crear un segundo eje Y para la GHI Medida
ax2 = ax1.twinx()
ax2.plot(filtered_data.index, filtered_data['GHI'], label='GHI Medida (W/m^2)', color='tab:orange')
ax2.set_ylabel('GHI (W/m^2)', color='tab:orange')
ax2.tick_params(axis='y', labelcolor='tab:orange')
ax2.set_ylim(0, filtered_data['GHI'].max()*1.1)  # Ajustar los límites del eje Y para la GHI Medida

# Añadir leyenda
fig.legend(loc='upper left', bbox_to_anchor=(0.1, 0.9))

# Títulos y leyendas
fig.suptitle('Simulación de Tracker y GHI Medida el 11 de junio en Madrid')

plt.show()
