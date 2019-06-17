import numpy as np
import matplotlib.pyplot as plt

plt.axis([0, 10, 0, 1])

for i in range(10):
    y = np.random.random()
    z = np.random.random()
    print(y)
    if (i > 10):
        plt.axis([i-10, i, 0, 1])
    plt.scatter(i, y, c="g", s=10) #, label= "g")
    plt.scatter(i, z, c="b")
    plt.pause(0.5)
    plt.legend()

#plt.show()