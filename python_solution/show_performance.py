import numpy as np
import matplotlib.pyplot as plt

matrix = np.loadtxt('./python_solution/o_notation_data.txt')

plt.plot(matrix[:,0],matrix[:,1])
plt.show()