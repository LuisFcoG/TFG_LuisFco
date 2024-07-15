# Tracker Model Data Processing
This repository contains programs for processing data campaigns from a model of a three-row solar tracker.

## Main Program Files (TMDP)
The main program files are responsible for the complete data processing workflow. The estudio.py file is the primary script that integrates the overall code and should be executed to perform a full processing cycle. The main program files are:

- datalogger.py: This library allows for initial processing of the data sheets.
- array_power.py: This script obtains the generated power values and creates the modules equivalent to the solar tracker model.
- csv_clear.py: This script removes all unnecessary information from the CSV files obtained from the measurements.
- training_data.py: This program is responsible for identifying the sweeps of the model and obtaining the maximum power point and the equivalent obtainable from the ephemeris equations.
- plottings.py: This code is responsible for graphically representing the most important information obtained from the processing.
- TMDP.py: This file integrates the general code and is the main script to be executed for complete processing.

## Theoretical Simulation Files
These files are used for theoretical simulations comparisons with certain measurements. They include:

- BifacialModelTest.py: Comparison between measured GHI data and modeled POA irradiance for a bifacial solar tracker system.
- Channel_Plot.py: Generates graphs of the specific measurement channels.
- GHI&POA.py: Compares and plots the theoretical values of GHI and POA.
- GHICompare.py: Simulation of the power output of a bifacial solar tracker system using clear-sky irradiance data and comparing it with measured GHI data.
- PowerSIM.py: Simulation of the power output of a bifacial solar tracker system, using measured GHI data and PVLib functions to calculate effective irradiance and DC power output.
- clear_csv.py: Independent program with the purpose of preparing CSV files for processing.
