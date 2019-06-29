import serial
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation
import time

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
sensor_id = {"sensor_1": 0, "sensor_2": 1, "sensor_3": 2}
sensor_color = ["b", "r", "g"]
sensor_last_data = [0, 0, 0]
sensor_last_time = [0, 0, 0]

def processData( row, j ):
    sensor, data = row.split(' ')
    sensor_last_data[sensor_id[sensor]] = data
    sensor_last_time[sensor_id[sensor]] = int(time.time())

# configura grafico
windows_size = 60
plt.axis([0, windows_size, 0, 45])
plt.xlabel("tempo (segundos)")
plt.ylabel("temperatura (celsius)")
plt.title("Monitoramento em Tempo Real com Módulos RF24")
plt.scatter(-1, 0, c="b", s=10, label="sensor_1")
plt.scatter(-1, 0, c="r", s=10, label="sensor_2")
#plt.scatter(-1, 0, c="g", s=10, label="sensor_3")
plt.legend()

# configura serial
master = serial.Serial('COM6', 115200)

# le serial
line = ""
j = 0
t = int(time.time())

while True:
    try:
        read = str(master.read()).split('\'')[1]
        if read == '\\r':
            processData(line, j)
            line = ""
            
            master.read() # remove o '\n'
        else:
            line = line + str(read)
    except EOFError:
        continue

    if int(time.time()) != t:
        t = int(time.time())
        if j >= windows_size:
            plt.axis([j-windows_size, j, 0, 45])
        for i in range(len(sensor_last_data)):
            if sensor_last_time[i] + 1 >= t:
                plt.scatter(j, int(sensor_last_data[i]), c=sensor_color[i], s=10)
            else:
                plt.scatter(j, -1, c=sensor_color[i], s=10)
        plt.pause(0.001)
        j += 1
    