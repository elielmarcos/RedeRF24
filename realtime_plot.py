import numpy as np
import matplotlib.pyplot as plt

plt.axis([0, 10, 0, 1])

#cria as legendas
plt.scatter(-1, 0, c="b", s=10, label="sensor_1")
plt.scatter(-1, 0, c="r", s=10, label="sensor_2")
plt.legend()

for i in range(10):
    y = np.random.random()
    z = np.random.random()
    print(y)
    if (i > 10):
        plt.axis([i-10, i, 0, 1])
    plt.scatter(i, y, c="b", s=10)
    plt.scatter(i, z, c="r", s=10)
    plt.pause(2)
    

#plt.show()