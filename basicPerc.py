# Basic Percolation code

import numpy as np
import random
from matplotlib import pyplot as plt
import timeit
# make error bars show up
plt.rcParams.update({'errorbar.capsize': 2})
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
	clustered = HK(lattice)
	return clustered

def HK(lattice):
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
	# copy = [[0 for x in range(lattice_size)] for y in range(lattice_size)]
	for x in range(lattice_size):
		for y in range(lattice_size):
			# copy[x][y] = label_list[clustered[x][y]]
			clustered[x][y] = label_list[clustered[x][y]]
	return clustered

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

# computes the error in the computation of an average
def get_error(values, num_resamples):
	if (len(values) == 0):
		average = 0
	else:
		average = sum(values)/len(values)
	num_samples = len(values)
	sampled_means = []
	for i in range(num_resamples):
		resampled = np.random.choice(values, num_samples)
		resampled_average = 0
		if (len(resampled) != 0):
			resampled_average = sum(resampled)/len(resampled) 
		sampled_means.append(resampled_average)
	# get the stdev of the resampled means:
	return np.std(sampled_means)

def plot_prob_vs_max(lattice_size, num_points, num_lattices, num_error_resamples):
	# plots the average maximum cluster size against the probability that a site will be occupied
	probabilities = [(n/num_points) for n in range(num_points)]
	y = [0 for x in range(num_points)]
	errors = [0 for x in range(num_points)]
	for x in range(num_points):
		maxes = [0 for i in range(num_lattices)]
		for i in range(num_lattices):
			maxes[i] = max_cluster_size(Perc(probabilities[x], lattice_size))
		y[x] = sum(maxes)/len(maxes)
		# error stuff will go here
		errors[x] = get_error(maxes, num_error_resamples)

	plt.errorbar(probabilities, y, yerr=errors,fmt='.')
	plt.title("Probability versus Max Cluster Size")
	plt.xlabel("Site Occupation Probability")
	plt.ylabel("Max Cluster Size (log)")
	plt.yscale('log')
	plt.show()

def plot_prob_vs_average(lattice_size, num_points, num_lattices, num_error_resamples):
	
	# plots the average cluster size against the probability of a site being occupied
	probabilities = [(n/num_points) for n in range(num_points)]
	averages = [0 for n in range(num_points)]
	error = [0 for n in range(num_points)]
	for i in range(num_points):
		# get all of the values in one array
		all_points = []
		for j in range(num_lattices):
			all_points.extend(get_cluster_sizes(Perc(probabilities[i], lattice_size)))

		# compute the average
		if (len(all_points) == 0):
			average = 0
		else:
			average = sum(all_points)/len(all_points)
		averages[i] = average
		# get error bars for that probability
		error[i] = get_error(all_points, num_error_resamples)
		print("point " + str(probabilities[i]) + " completed")
	#plot it
	plt.errorbar(probabilities, averages, yerr=error, fmt='.')
	plt.title("Probability versus Average Cluster Size")
	plt.xlabel("Site Occupation Probability")
	plt.ylabel("Average Cluster Size (log)")
	plt.yscale('log')
	plt.show()

#gets the raw data for averages, which can then all be plotted on one big chart
def get_average_data(lattice_size, num_points, num_lattices, num_error_resamples):
	probabilities = [(n/num_points) for n in range(num_points)]
	averages = [0 for n in range(num_points)]
	error = [0 for n in range(num_points)]
	for i in range(num_points):
		# get all of the values in one array
		all_points = []
		for j in range(num_lattices):
			all_points.extend(get_cluster_sizes(Perc(probabilities[i], lattice_size)))

		# compute the average
		if (len(all_points) == 0):
			average = 0
		else:
			average = sum(all_points)/len(all_points)
		averages[i] = average
		# get error bars for that probability
		error[i] = get_error(all_points, num_error_resamples)
	return (probabilities, averages, error)


def plot_diff_lattice_sizes(lattice_sizes, num_points, num_lattices, num_error_resamples):
	probabilities = [(n/num_points) for n in range(num_points)]
	average_list = [[] for n in range(len(lattice_sizes))]
	error_list = [[] for n in range(len(lattice_sizes))]

	# get the data for each lattice size
	for i in range(len(lattice_sizes)):
		temp = get_average_data(lattice_sizes[i], num_points, num_lattices, num_error_resamples)
		average_list[i] = temp[1]
		error_list[i] = temp[2]
		plt.errorbar(probabilities, average_list[i], yerr=error_list[i], fmt='.')
		print("Completed lattice size " + str(lattice_sizes[i]))

	plt.title("Probability versus Average Cluster Size")
	plt.xlabel("Site Occupation Probability")
	plt.ylabel("Average Cluster Size (log)")
	legend_labels = [0 for n in range(len(lattice_sizes))]
	for i in range(len(lattice_sizes)):
		legend_labels[i] = "Lattice Size " + str(lattice_sizes[i])
	plt.legend(legend_labels, loc='lower right')
	plt.yscale('log')
	plt.show()
# plots the averages for a certain range of probabilities
def plot_probability_range(lattice_size, num_points, num_lattices, num_error_resamples, prob_low_end, prob_high_end):
	# plots the average cluster size against the probability of a site being occupied
	probabilities = [prob_low_end for n in range(num_points)]
	# initialize the probability array to fit the range
	step  = (prob_high_end - prob_low_end)/num_points
	for i in range(num_points):
		if (i != 0):
			probabilities[i] = probabilities[i - 1] + step

	averages = [0 for n in range(num_points)]
	error = [0 for n in range(num_points)]
	for i in range(num_points):
		# get all of the values in one array
		all_points = []
		for j in range(num_lattices):
			all_points.extend(get_cluster_sizes(Perc(probabilities[i], lattice_size)))

		# compute the average
		if (len(all_points) == 0):
			average = 0
		else:
			average = sum(all_points)/len(all_points)
		averages[i] = average
		# get error bars for that probability
		error[i] = get_error(all_points, num_error_resamples)
		print("point " + str(probabilities[i]) + " completed")
	#plot it
	plt.errorbar(probabilities, averages, yerr=error, fmt='.')
	plt.title("Probability versus Average Cluster Size")
	plt.xlabel("Site Occupation Probability")
	plt.ylabel("Average Cluster Size (log)")
	plt.yscale('log')
	plt.show()

def main():
	#plot_prob_vs_max(20,50,20,100)
	#plot_prob_vs_average(10,50,20,100)
	#plot_diff_lattice_sizes([5, 20, 50, 100], 50, 20, 100)
	plot_probability_range(1000, 5, 1, 100, .5, .7)

main()	
