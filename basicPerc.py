# Basic Percolation code

import numpy as np
import random
from matplotlib import pyplot as plt

# Gonna do it in 2D first
# lattice_size is the length of the square lattice
# probability is the probability that a site will be on/off (0-1) 
def Perc(probability, lattice_size):
	# 0 is blocked and 1 is open
	global labels
	labels = [n for n in range(int((lattice_size * lattice_size)/2))]
	lattice = [[0 for x in range(lattice_size)] for y in range(lattice_size)]
	# set up the lattice
	for x in range(lattice_size):
		for y in range(lattice_size):
			# set the site to on
			if (random.random() < probability):
				lattice[x][y] = 1

	# Using Hoshen-Kopelman alg to label the clusters
	clustered = HK2(lattice)
	return clustered


def find(x):
	global labels
	return labels[x]


def union(x, y):
	global labels
	global labeled
	for i in range(len(labels)):
		if (labels[i] == labels[x]):
			labels[i] = labels[y]
	return y


# Runs the Hoshen-Kopelman algorithm
def HK(lattice):
	lattice_size = len(lattice)
	global labels
	global labeled
	labeled = [[0 for x in range(lattice_size)] for y in range(lattice_size)]
	largest_label = 0
	# for every point
	for x in range(lattice_size):
		for y in range(lattice_size):
			# if the point is occupied
			if (lattice[x][y] == 1):
				
				# get the values of the point to its left and above it
				left = labeled[x][y-1]
				above = labeled[x-1][y]
				if (x - 1 < 0):
					above = 0
				if (y - 1 < 0):
					left = 0
				# if neither are labeled
				if ((left == 0) and (above == 0)):
					# make a new cluster for this point
					largest_label += 1
					labeled[x][y] = largest_label
				# the left one is labeled and the one above is not
				elif ((left != 0) and (above == 0)):
					# use the label of the one to the left
					labeled[x][y] = left
				# the cell above is labeled
				elif ((left == 0) and (above != 0)):
					# use the label of the one above
					labeled[x][y] = above
				#Both are labeled so we have to use union
				else:
					labeled[x][y] = union(left, above)

	for x in range(lattice_size):
		for y in range(lattice_size):
			labeled[x][y] = labels[labeled[x][y]]
	return labeled

def HK2(lattice):
	lattice_size = len(lattice)
	global label_list
	label_list = [n for n in range(int((lattice_size * lattice_size)/2))]
	clustered = [[0 for x in range(lattice_size)] for y in range(lattice_size)]
	index = 0
	for x in range(lattice_size):
		for y in range(lattice_size):
			if (lattice[x][y] == 1):
				left_label_index = clustered[x][y-1]
				above_label_index = clustered[x-1][y]
				if (x - 1 < 0):
					above_label_index = 0
				if (y - 1 < 0):
					left_label_index = 0
				
				if ( (left_label_index == 0) and (above_label_index == 0) ):
					index += 1
					clustered[x][y] = index
				elif ( (left_label_index != 0) and (above_label_index == 0) ):
					clustered[x][y] = left_label_index
				elif ( (left_label_index == 0) and (above_label_index != 0) ):
					clustered[x][y] = above_label_index
				else:
					# union time
					label_list_copy = label_list.copy()
					for i in range(len(label_list)):
						if (label_list[i] == label_list[above_label_index]):
							label_list_copy[i] = label_list[left_label_index]
						clustered[x][y] = left_label_index
					label_list = label_list_copy.copy()
	copy = [[0 for x in range(lattice_size)] for y in range(lattice_size)]
	for x in range(lattice_size):
		for y in range(lattice_size):
			copy[x][y] = label_list[clustered[x][y]]
	return copy

# gets the number of clusters after running HK
def count_clusters(clustered):
	# cast the clustered array to a numpy array
	new_array = np.asarray(clustered)
	# now get the number of unique elements in the numpy array
	# (the subtraction is to get rid of 0)
	return len(np.unique(new_array))-1
# returns a list with the number of items in each cluster
def get_cluster_sizes(clustered):
	ret = [-1 for n in range(len(clustered) ** 2)]
	for x in range(len(clustered)):
		for y in range(len(clustered)):
			val = clustered[x][y]
			ret[val] += 1
	# prune the 0 label:
	del ret[0]

	# prune all the labels with only 0s:
	ret = [i for i in ret if i != -1]
	return ret


# returns the average cluster size in a lattice
def get_average_cluster_size(clustered):
	cluster_sizes = get_cluster_sizes(clustered)
	if len(cluster_sizes) == 0:
		return 0
	average = sum(cluster_sizes)/count_clusters(clustered)
	return average

# gets the maximum cluster size in a lattice
def max_cluster_size(clustered):
	if len(get_cluster_sizes(clustered)) == 0:
		return 0
	return max(get_cluster_sizes(clustered))

def display_lattice(clustered):
	clustered = np.asarray(clustered)
	clustered = np.ma.masked_where(clustered == 0, clustered)
	cmap = plt.get_cmap('RdBu', np.max(clustered)-np.min(clustered)+1)
	cmap.set_bad(color='black')
	mat = plt.matshow(clustered,cmap=cmap,vmin = np.min(clustered)-.5, vmax = np.max(clustered)+.5)
	cax = plt.colorbar(mat, ticks=np.arange(np.min(clustered),np.max(clustered)+1))
	plt.show()

def main():

	lattice_size = 20
	# while lattice_size < 100:
	# 	#Number of lattices we want to test
	# 	num_runs = 50
	# 	# number of lattices per probability
	# 	num_lattices = 10
	# 	# the different probabilities that we're testing	
	# 	probabilities = [(n/num_runs) for n in range(num_runs)]
	# 	y = [get_average_cluster_size(Perc(probabilities[n], lattice_size)) for n in range(num_runs)]
	# 	plt.scatter(probabilities, y)
	# 	plt.xlabel("Site Occupation Probability")
	# 	plt.ylabel("Max Cluster Size")	
	# 	lattice_size += 10
	# plt.show()


	#showing a lattice as an image:
	clustered = Perc(.65, lattice_size)
	print(max_cluster_size(clustered))
	print(get_average_cluster_size(clustered))
	display_lattice(clustered)



main()	
