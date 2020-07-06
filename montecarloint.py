import numpy as np
from matplotlib import pyplot as plt
import random

# first define an arbitrary function
def func(x):
	return x**3


def mcint(x_1, x_2, num_points):
	# compute y_1 and y_2
	y_1 = func(x_1)
	y_2 = func(x_2)
	# take the max of y_1 and y_2 and make that the height of the box
	box_height = max(y_1, y_2)
	# compute the area of the box
	area = box_height * (x_2 - x_1)
	# randomly pick points till oblivion
	inside = 0
	for point in range(num_points):
		# pick a random point, given an x and y value
		point_x = random.uniform(x_1, x_2)
		point_y = random.uniform(0, box_height)
		# if its inside the function, add 1 to the inside counter
		if point_y < func(point_x):
			inside += 1

	# compute ratio
	ratio = inside / num_points

	return ratio * area 



print(mcint(-2, 10, 10000))
