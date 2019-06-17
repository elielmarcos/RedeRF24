import serial
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation

# cria base de dados
sensors = {"sensor_1": "b", "sensor_2": "r", "sensor_3": "g"}
datas = []
size = 10
for i in range(len(sensors)):
    row = []
    for i in range(size):
        row.append(0)
    datas.append(row)

# processa linha de entrada
sensor_color = {"sensor_1": "b", "sensor_2": "r", "sensor_3": "g"}
windows_size = 500
def processData( row, j ):
    sensor, data = row.split(' ')
    if j >= windows_size:
        plt.axis([j-windows_size, j, 0, 45])
    plt.scatter(j, int(data), c=sensor_color[sensor], s=10)
    plt.pause(0.001)

# configura grafico
plt.axis([0, windows_size, 0, 45])
plt.scatter(-1, 0, c="b", s=10, label="sensor_1")
plt.scatter(-1, 0, c="r", s=10, label="sensor_2")
plt.scatter(-1, 0, c="g", s=10, label="sensor_3")
plt.legend()

# configura serial
master = serial.Serial('COM3', 115200)

# le serial
line = ""
j = 0
while True:
    try:
        read = str(master.read()).split('\'')[1]
        if read == '\\r':
            processData(line, j)
            line = ""
            j += 1
            master.read() # remove o '\n'
        else:
            line = line + str(read)
    except EOFError:
        continue