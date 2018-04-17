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
fuel_average_coeffs = [1439.57, 1482.51, 1511.48, 1531.59, 1545.76, 1555.95,
1563.34, 1568.85, 1573.03, 1576.25, 1578.77, 1580.83, 1582.59, 1584.04,
1585.30, 1586.41, 1587.42, 1588.30]
normalization = 1.0 / np.sqrt(2.0 * math.pi)
fuel_average_temps = [i * normalization for i in fuel_average_coeffs]
fuel_average_temps_relative_change = np.zeros(len(fuel_average_temps))

# k_eff (track-length)
k_eff = [0.421719, 0.422127, 0.421879, 0.421812, 0.421915, 0.421821, 0.42172, 0.421708, 0.421772, 0.421711, 0.421784, 0.421817, 0.421824, 0.421715, 0.421949, 0.421899, 0.421895, 0.422068]
k_eff_absolute_change = np.zeros(len(k_eff))
k_eff_relative_change = np.zeros(len(k_eff))

for i in range(1, len(k_eff)):
        k_eff_relative_change[i] = np.abs(k_eff[i] - k_eff[i - 1]) / k_eff[i - 1]
        k_eff_absolute_change[i] = np.abs(k_eff[i] - k_eff[i - 1])
        fuel_average_temps_relative_change[i] = np.abs(fuel_average_temps[i] - fuel_average_temps[i - 1]) / fuel_average_temps[i]

fission_distribution = np.zeros((len(k_eff), num_Zernike))
fission_distribution[0, :] = (1.86772e+07, 12449.1, -1.48766e+06, -10169, 72822.6, -879879, 3092.37, -5535.34, -60986.7, -307155, 8553.63, 558.366, 56976.6, -77558, -95028, 457.528, 2710.82, -4701, 858.016, -38431, 114884, -14353.3, -3374.94, 3712.83, -39642.7, 21163.2, -11713.2, 81546.9)
fission_distribution[1, :] = (1.86985e+07, -11415.5, -1.48688e+06, 1069.34, 62693.9, -865043, -5424.01, 876.318, -56269.3, -321145, -5509.35, -4747.37, 63485.4, -71465.2, -90257.3, -4394.98, -4780.25, 6528.51, -16033.7, -53364.2, 117758, 4823.69, 6644.55, 2089.21, -37899.1, 4429.43, -18406, 86270.1)
fission_distribution[2, :] = (1.86793e+07, -4024.75, -1.48474e+06, -14315.9, 64506.3, -875612, 14566.4, 4782.16, -62249.8, -318467, -2566.22, 11978.9, 76229.2, -78696.2, -93792.5, -9560.16, -2789.79, 1296.25, -118.875, -44478.1, 111818, 1550.86, -7342.13, 5294.45, -33367.8, 20018.2, -20693.8, 67572.9)
fission_distribution[3, :] = (1.86846e+07, -5728.31, -1.48819e+06, 3333.78, 68372.8, -857097, -1097.6, -3681, -60385.3, -321964, -5507.88, -10994.4, 77474.9, -67018.7, -97674.8, 3675.44, 911.602, 2559.28, -14981.5, -55670, 106678, 3608.07, 717.454, -4560.64, -40642.4, 12882.2, -16418.5, 79130.7)
fission_distribution[4, :] = (1.86716e+07, -3083.63, -1.48338e+06, 5729.75, 64547.9, -875216, 2588.76, -1388.48, -61801.2, -321758, -4851.47, -3498.74, 70644.1, -62558.6, -87237.5, 20313.5, -913.266, -2551.95, -11515.6, -51186.2, 101826, 2358.64, 10372.3, 7684.41, -41282.7, 19516.5, -23997.9, 81046.6)
fission_distribution[5, :] = (1.86845e+07, -6078.49, -1.50164e+06, -9094.84, 70326.3, -863574, -2444.41, -6195.36, -60391.1, -315169, -2339.58, 7430.67, 75010, -68856.6, -90619.2, -1070.57, -7348.31, -3195.11, -2345.05, -33371.8, 114568, 1664.41, -8628.38, -11474.5, -44941.5, 18904, -17080.2, 80249.8)
fission_distribution[6, :] = (1.86791e+07, -6270.54, -1.49232e+06, -6087.77, 79094.4, -868517, -6530.59, 1170.77, -57678.3, -319079, 2499.11, 7048.65, 72741.4, -69845.3, -102783, -5326.95, 962.636, 6111.5, 3522.89, -47742.5, 104357, 9275.77, -3992.05, 82.3932, -46982, 19129.1, -21059.2, 82726.5)
fission_distribution[7, :] = (1.86805e+07, 2419.65, -1.49026e+06, -7014.91, 64324.6, -865304, 15365.6, 4530.5, -68377.4, -317719, -5183.91, -6173.39, 79948.3, -70609.6, -97860.5, -5315.75, -4775.9, 5733.06, 1395.88, -46124.1, 111140, 7566.72, -601.264, 9914.73, -34005.7, 20641, -10132.3, 79909.6)
fission_distribution[8, :] = (1.86868e+07, 246.504, -1.48643e+06, 12563.2, 68823.5, -869013, 790.508, 9311.88, -53669.8, -311987, 10569.3, -724.949, 69479.4, -74068.1, -94135.9, -1860.26, -8156.87, -9006.99, -11313.5, -54009.4, 105116, -5419.25, 5089.02, 6663.76, -30288.6, 19437.7, -10826.1, 88166.6)
fission_distribution[9, :] = (1.86832e+07, -1594.18, -1.47434e+06, -76.7145, 64918.3, -873405, -98.1967, -1411.1, -66669.3, -317823, -4207.23, 1639.69, 69823.6, -59543.4, -103386, 9457.31, 1645.11, -1699.32, -9808.61, -57914.6, 108194, -4062.43, 2686.55, -923.993, -36034.5, 14084.3, -27128.8, 83236)
fission_distribution[10, :] = (1.86799e+07, -727.453, -1.49321e+06, 5722.53, 84747.5, -870741, 4282.13, 3120.76, -54079.9, -321019, 1963.3, -9176.73, 60015.2, -66844, -95260.6, 1091.74, -9611.42, -4859.96, 3877.49, -43720.9, 108811, -6658.91, -925.896, 4622.89, -30487.7, -2613.39, -31882.7, 79400)
fission_distribution[11, :] = (1.86794e+07, -1592.77, -1.48527e+06, 4161.12, 69496.3, -867846, 1905.36, -3189.36, -68353.1, -323094, 1328.14, -257.742, 74388, -71999, -92630.3, -6455.22, -3061.33, -3319.52, 2824.85, -38228.1, 113201, 9598.49, 7344.29, 4025.45, -37811.3, 27356.2, -22121.1, 80752.3)
fission_distribution[12, :] = (1.86955e+07, -3056.89, -1.49278e+06, 1594.52, 74821.5, -866594, 91.4048, 7598.9, -61192.7, -317565, -2576.95, -7194.88, 67396.8, -69186.6, -93715.5, 9921.8, 4108.15, -3258.48, -16630.6, -53884.2, 98726.5, 3004.53, 4042.61, -1800.44, -45241.5, 22989, -8483.57, 86079.4)
fission_distribution[13, :] = (1.86838e+07, -2639.49, -1.49403e+06, -4723.08, 70033, -868060, -1814.44, -5418.7, -56717.8, -314570, 16186, 3771.66, 64436.6, -76533, -97021.4, 2081.44, -515.023, -3447.73, -7111.49, -49107.1, 101263, -5145.72, -2229.72, 73.5906, -27636.3, 4576.2, -32476.1, 82266.3)
fission_distribution[14, :] = (1.86892e+07, -5589.98, -1.48784e+06, 9459.11, 78491, -861880, -3901.28, -2472.97, -67027.5, -317894, 13495.5, 12562.3, 68668.1, -67288.8, -95354.6, -11881.8, -12999.6, -7886.56, 702.862, -47743.4, 109691, -492.987, -2930.91, -3289.75, -37070.9, 12245.8, -21235.7, 75717.3)
fission_distribution[15, :] = (1.86922e+07, -899.186, -1.47996e+06, -1808.58, 77947.2, -878259, 5690.32, 4417.46, -65194.5, -318666, 11953.3, 1543.14, 63242.5, -69731.8, -92345.4, -8476.86, 4136.75, 5606.91, -2889.94, -35785.6, 113946, 7106.39, 4068.18, 639.482, -27697.1, 23240.8, -19423.2, 75973.2)
fission_distribution[16, :] = (1.86976e+07, -817.755, -1.50019e+06, 6799.24, 72849, -867574, -12029.7, 3725.91, -60035, -318573, -2442.99, 1791.85, 70060.7, -53820.1, -86168, 16254.3, -934.64, -7476.61, 5402.33, -37227.7, 104815, 41.4137, -1488.4, 6135.81, -35934.4, 7338.01, -15201, 74638.4)
fission_distribution[17, :] = (1.86898e+07, 3467.57, -1.47627e+06, -2050.93, 79261.8, -868212, -3706.82, 1300.27, -62279.8, -317535, 1586.62, -6328.81, 58038.8, -69473.7, -97470.8, -9709.4, -3996.16, 174.866, -14382.9, -65012.5, 110913, 11309.7, -408.375, -6240.92, -27685.2, 23499.5, 160.072, 89390.2)

fluid_layer_temps = np.zeros((18, num_fluid_layers))
fluid_layer_temps[0, :] = (549.396, 548.986, 548.932, 548.928)
fluid_layer_temps[1, :] = (549.003, 548.093, 547.884, 547.845)
fluid_layer_temps[2, :] = (548.71, 547.332, 546.883, 546.778)
fluid_layer_temps[3, :] = (548.471, 546.672, 545.945, 545.742)
fluid_layer_temps[4, :] = (548.267, 546.086, 545.073, 544.748)
fluid_layer_temps[5, :] = (548.087, 545.56, 544.263, 543.803)
fluid_layer_temps[6, :] = (547.925, 545.082, 543.511, 542.912)
fluid_layer_temps[7, :] = (547.779, 544.644, 542.812, 542.075)
fluid_layer_temps[8, :] = (547.645, 544.242, 542.163, 541.292)
fluid_layer_temps[9, :] = (547.522, 543.87, 541.56, 540.561)
fluid_layer_temps[10, :] = (547.409, 543.527, 540.999, 539.88)
fluid_layer_temps[11, :] = (547.305, 543.208, 540.478, 539.245)
fluid_layer_temps[12, :] = (547.208, 542.913, 539.994, 538.655)
fluid_layer_temps[13, :] = (547.118, 542.639, 539.544, 538.105)
fluid_layer_temps[14, :] = (547.035, 542.385, 539.126, 537.594)
fluid_layer_temps[15, :] = (546.957, 542.148, 538.737, 537.118)
fluid_layer_temps[16, :] = (546.885, 541.929, 538.376, 536.676)
fluid_layer_temps[17, :] = (546.818, 541.725, 538.04, 536.265)

iterations = np.linspace(0, len(k_eff) - 1, len(k_eff))
centroids = (0.125, 0.375, 0.625, 0.875)

# plot the fluid layer temperatures for several Picard iterations
fig = plt.figure()
ax1 = fig.add_subplot(111)
ax1.plot(centroids, (550.0, 550.0, 550.0, 550.0), 'ko-', label='Iteration 0')
ax1.plot(centroids, fluid_layer_temps[5, :], 'ro-', label='Iteration 5')
ax1.plot(centroids, fluid_layer_temps[10, :], 'mo-', label='Iteration 10')
ax1.plot(centroids, fluid_layer_temps[15, :], 'bo-', label='Iteration 15')
ax1.plot(centroids, fluid_layer_temps[16, :], 'go-', label='Iteration 16')
ax1.plot(centroids, fluid_layer_temps[17, :], 'co-', label='Iteration 17')

ax1.set_xlabel('Layer centroid (cm)', fontsize=axis_font_size)
ax1.set_ylabel('Layer-averaged temperature (K)', fontsize=axis_font_size)
ax1.set_xlim([0, 1])
ax1.set_ylim([536, 551])
ax1.tick_params(labelsize=tick_font_size)
ax1.legend(loc = 3, fontsize=font_size)
plt.savefig('layer_temps.pdf', bbox_inches='tight')



fig = plt.figure()
ax1 = fig.add_subplot(111)

plt1 = ax1.plot(iterations, fuel_average_temps, 'ro', label='value')
ax1.xaxis.set_major_formatter(FormatStrFormatter('%.0f'))
ax1.set_xlabel('Picard iteration number', fontsize=axis_font_size)
ax1.set_ylabel('Fuel average temperature (K)', fontsize=axis_font_size)
ax1.tick_params('y', labelsize=tick_font_size, colors='r')
ax1.yaxis.set_major_formatter(FormatStrFormatter('%.0f'))

ax2 = ax1.twinx()
plt2 = ax2.plot(iterations[1:], fuel_average_temps_relative_change[1:], 'bo-', label='relative change')
ax2.set_ylabel('Relative change', fontsize=axis_font_size)
ax2.tick_params('y', labelsize=tick_font_size, colors='b')

legends = plt1 + plt2
labs = [l.get_label() for l in legends]
ax1.legend(legends, labs, loc=0, fontsize=font_size)
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

