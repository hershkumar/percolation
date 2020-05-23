import numpy as np
import random
from matplotlib import pyplot as plt
lattice_size = 5
lattice = [[x for x in range(lattice_size)] for y in range(lattice_size)]

for x in range(lattice_size):
	for y in range(lattice_size):
		print(str(lattice[x][y]) + " ")