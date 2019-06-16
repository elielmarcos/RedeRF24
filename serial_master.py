import serial
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation

# cria base de dados
sensors = {"sensor_1": 0, "sensor_2": 1, "sensor_3": 2}
datas = []
size = 10
for i in range(len(sensors)):
    row = []
    for i in range(size):
        row.append(0)
    datas.append(row)

# processa linha de entrada
def processData( row ):
    sensor, data = row.split(' ')
    sensor_id = sensors[sensor]
    datas[sensor_id].append(int(data))
    datas[sensor_id].pop(0)
    print("sensor_" + str(sensor_id) + ": " + str(datas[sensor_id]))
    #plt.draw() 
    #plt.pause(0.01)

# configura grafico
tempo = np.arange(0, size, 1)
fig, ax = plt.subplots()
plt.xlim(0,10) 
plt.ylim(0,10)
#ax.plot(tempo, datas[0])

# configura serial
master = serial.Serial('COM3', 115200)

# le serial
line = ""
while True:
    try:
        read = str(master.read()).split('\'')[1]
        if read == '\\r':
            processData(line)
            line = ""
            master.read() # remove o '\n'
        else:
            line = line + str(read)
    except EOFError:
        continue