# these are the results that are actually used in the paper

import numpy as np
import math as math
import matplotlib.pyplot as plt
from matplotlib import rc
from matplotlib.ticker import ScalarFormatter, FormatStrFormatter


rc('mathtext', default = 'regular')

font_size=16
axis_font_size = 18
tick_font_size=15

Zernike_order = 6
num_Zernike = 0.5 * (Zernike_order + 1.0) * (Zernike_order + 2.0)
num_fluid_layers = 4

# fuel average temperatures (the zeroth-order coefficient computed by BISON)
# these coefficients need to be normalized to obtain units of temperature.
fuel_average_coeffs = [1439.62, 1482.52, 1511.53, 1531.62, 1545.81, 1555.94, 1563.32, 1568.77, 1572.89, 1576.12, 1578.65, 1580.67, 1582.34, 1583.81]
normalization = 1.0 / np.sqrt(2.0 * math.pi)
fuel_average_temps = [i * normalization for i in fuel_average_coeffs]
fuel_average_temps_relative_change = np.zeros(len(fuel_average_temps))

# k_eff (track-length)
k_eff = [0.421938, 0.422131, 0.421846, 0.421886, 0.421681, 0.421513, 0.421467, 0.421501, 0.421384, 0.421256, 0.421331, 0.421347, 0.421303]
k_eff_absolute_change = np.zeros(len(k_eff))
k_eff_relative_change = np.zeros(len(k_eff))

for i in range(1, len(k_eff)):
        k_eff_relative_change[i] = np.abs(k_eff[i] - k_eff[i - 1]) / k_eff[i - 1]
        k_eff_absolute_change[i] = np.abs(k_eff[i] - k_eff[i - 1])
        fuel_average_temps_relative_change[i] = np.abs(fuel_average_temps[i] - fuel_average_temps[i - 1]) / fuel_average_temps[i]

fluid_layer_temps = np.zeros((len(k_eff) + 3, num_fluid_layers))
fluid_layer_temps[0, :] = (550, 550, 550, 500)
fluid_layer_temps[1, :] =  (550.604, 551.014, 551.068, 551.072)
fluid_layer_temps[2, :] = (550.997, 551.907, 552.116, 552.155)
fluid_layer_temps[3, :] = (551.29, 552.668, 553.117, 553.222)
fluid_layer_temps[4, :] = (551.529, 553.328, 554.055, 554.258)
fluid_layer_temps[5, :] = (551.733, 553.914, 554.927, 555.252)
fluid_layer_temps[6, :] = (551.913, 554.44, 555.737, 556.197)
fluid_layer_temps[7, :] = (552.075, 554.918, 556.489, 557.088)
fluid_layer_temps[8, :] = (552.221, 555.356, 557.188, 557.925)
fluid_layer_temps[9, :] = (552.355, 555.758, 557.837, 558.708)
fluid_layer_temps[10, :] = (552.478, 556.13, 558.44, 559.439)
fluid_layer_temps[11, :] = (552.591, 556.473, 559.001, 560.12)
fluid_layer_temps[12, :] = (552.695, 556.792, 559.522, 560.755)
fluid_layer_temps[13, :] = (552.792, 557.087, 560.006, 561.345)
fluid_layer_temps[14, :] = (552.882, 557.361, 560.456, 561.895)
fluid_layer_temps[15, :] = (552.965, 557.615, 560.874, 562.406)

iterations = np.linspace(0, len(k_eff) - 1, len(k_eff))
centroids = (0.125, 0.375, 0.625, 0.875)

# plot the fluid layer temperatures for several Picard iterations
fig = plt.figure()
ax1 = fig.add_subplot(111)
ax1.plot(centroids, (550.0, 550.0, 550.0, 550.0), 'ko-', label='Iteration 0')
ax1.plot(centroids, fluid_layer_temps[5, :], 'rv-', label='Iteration 5')
ax1.plot(centroids, fluid_layer_temps[8, :], 'm^-', label='Iteration 8')
ax1.plot(centroids, fluid_layer_temps[10, :], 'b*-', label='Iteration 10')
ax1.plot(centroids, fluid_layer_temps[11, :], 'gD-', label='Iteration 11')
ax1.plot(centroids, fluid_layer_temps[12, :], 'c8-', label='Iteration 12')

ax1.set_xlabel('Layer centroid (cm)', fontsize=axis_font_size)
ax1.set_ylabel('Layer-averaged temperature (K)', fontsize=axis_font_size)
ax1.set_xlim([0, 1])
ax1.set_ylim([548, 564])
ax1.tick_params(labelsize=tick_font_size)
ax1.legend(loc = 2, fontsize=font_size)
plt.savefig('layer_temps.pdf', bbox_inches='tight')



fig = plt.figure()
ax1 = fig.add_subplot(111)
plt1 = ax1.plot(iterations, fuel_average_temps[1:14], 'ro', label='value')
ax1.xaxis.set_major_formatter(FormatStrFormatter('%.0f'))
ax1.set_xlabel('Picard iteration number', fontsize=axis_font_size)
ax1.set_ylabel('Fuel average temperature (K)', fontsize=axis_font_size)
ax1.tick_params('y', labelsize=tick_font_size, colors='r')
ax1.yaxis.set_major_formatter(FormatStrFormatter('%.0f'))

ax2 = ax1.twinx()
print(len(iterations[1:]), len(fuel_average_temps_relative_change[1:13]))
plt2 = ax2.plot(iterations[1:], fuel_average_temps_relative_change[1:13], 'bo-', label='relative change')
ax2.set_ylabel('Relative change', fontsize=axis_font_size)
ax2.tick_params('y', labelsize=tick_font_size, colors='b')

legends = plt1 + plt2
labs = [l.get_label() for l in legends]
ax1.legend(legends, labs, loc=3, fontsize=font_size)
plt.savefig('temp.pdf', bbox_inches='tight')




# plot k_eff and its relative change as a function of Picard iteration number
fig = plt.figure()
ax1 = fig.add_subplot(111)

plt1 = ax1.plot(iterations, k_eff, 'ro', label='value')
ax1.xaxis.set_major_formatter(FormatStrFormatter('%.0f'))
ax1.set_xlabel('Picard iteration number', fontsize=axis_font_size)
ax1.set_ylabel('Infinite multiplication factor', fontsize=axis_font_size)
ax1.tick_params('y', labelsize=tick_font_size, colors='r')
ax1.yaxis.set_major_formatter(FormatStrFormatter('%.5f'))

ax2 = ax1.twinx()
plt2 = ax2.plot(iterations[1:], 100000 * k_eff_absolute_change[1:], 'bo-', label='absolute change')
ax2.set_ylabel('Absolute change (pcm)', fontsize=axis_font_size)
ax2.tick_params('y', labelsize=tick_font_size, colors='b')

legends = plt1 + plt2
labs = [l.get_label() for l in legends]
ax1.legend(legends, labs, loc=0, fontsize=font_size)
plt.savefig('k_eff.pdf', bbox_inches='tight')

