from matplotlib import pyplot as plt
import numpy as np

def main():
	file = open("walk_data.txt")
	x = []
	y = []


	for line in file:

		split = list(map(float, line.split(" ")))
		x.append(split[0])
		#print(split[0])
		y.append(split[1])
		#print(split[1])

	plt.plot(x,y)
	plt.title("Average Cluster Size vs Site Probability")
	plt.ylabel("Distance from Origin")
	plt.xlabel("Step Number")
	plt.show()


	file.close()	



main()
