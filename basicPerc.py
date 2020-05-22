# Basic Percolation code

import numpy as np
import random
import pandas as pd
from matplotlib import pyplot as plt

# Gonna do it in 2D first
# lattice_size is the length of the square lattice
# probability is the probability that a site will be on/off (0-1) 
def Perc(probability, lattice_size):
	# 0 is blocked and 1 is open
	lattice = [[0 for x in range(lattice_size)] for y in range(lattice_size)]
	# set up the lattice
	for x in range(lattice_size):
		for y in range(lattice_size):
			# set the site to on
			if (random.random() < probability):
				lattice[x][y] = 1

	# Using Hoshen-Kopelman alg to label the clusters
	clustered = HK(lattice)
	return clustered


def find(x):
	global labels
	y = x
	while (labels[y] != y):
		y = labels[y]
	while (labels[x] != x):
		z = labels[x]
		labels[x] = y
		x = z
	return y


def union(x, y):
	global labels
	labels[find(x)] = find(y)


def HK(lattice):
	lattice_size = len(lattice)
	global labels
	labels = [n for n in range(lattice_size * lattice_size)]
	label =  [[0 for x in range(lattice_size)] for y in range(lattice_size)]
	largest_label = 0
	# for every point
	for x in range(lattice_size):
		for y in range(lattice_size):
			# if the point is occupied
			if (lattice[x][y] == 1):
				# get the values of the point to its left and above it
				left = lattice[x-1][y]
				above = lattice[x][y-1]
				# if neither are labeled
				if ((left == 0) and (above == 0)):
					# make a new cluster for this point
					largest_label += 1
					label[x][y] = largest_label
				# the left one is labeled and the one above is not
				elif ((left != 0) and (above == 0)):
					label[x][y] = find(left)
				# the cell above is labeled
				elif ((left == 0) and (above != 0)):
					label[x][y] = find(above)
				else:
					union(left, above)
					label[x][y] = find(left)
	return label

def count_clusters(clustered):
	new_array = np.asarray(clustered)
	# now get the number of unique elements in the numpy array
	# (the subtraction is to get rid of 0)
	return len(np.unique(new_array))-1

def main():
	lattice_size = 100
	num_runs = 1000
	probabilities = [(n/num_runs) for n in range(num_runs)]
	y = [count_clusters(Perc(probabilities[n], lattice_size)) for n in range(num_runs)]
	plt.scatter(probabilities, y)
	plt.xlabel("Site Occupation Probability")
	plt.ylabel("Number of Clusters")
	plt.show()
	##Saving to txt file
	#pd.set_option('display.max_columns', None)
	#pd.set_option('display.max_rows', None)
	#np.savetxt(r'table.txt', pd.DataFrame(clustered).values, fmt='%d')



main()	
