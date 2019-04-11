
# Written by: 	Suren Gourapura
# Written on: 	March 1, 2019
# Purpose: 	Plot the 2D gain and phase pattern

import numpy as np		# for data manipulation, storage
import matplotlib.pyplot as plt	# For plotting
#import os			# exclusively for rstrip()
import argparse			# for getting the user's arguments from terminal
# May be needed: from mpl_toolkits.mplot3d import Axes3D 

#---------GLOBAL VARIABLES----------GLOBAL VARIABLES----------GLOBAL VARIABLES----------GLOBAL VARIABLES


# We need to grab the three arguments from the bash script or user. These arguments in order are [the name of the source folder of the fitness scores], [the name of the destination folder for the plots], and [the number of generations]
parser = argparse.ArgumentParser()
parser.add_argument("numGens", help="Number of generations the code is running for", type=int)
parser.add_argument("NPOP", help="Number of individuals in a population", type=int)
g = parser.parse_args()

# The name of the data file
dataFile = "XF_data.txt"
# The name of the plot that will be put into the destination folder
plotName = "LRPlot2D"
filePath = "/home/suren/Desktop/OSU_Research/Carls_Plotter/"


#----------DEFINITIONS HERE----------DEFINITIONS HERE----------DEFINITIONS HERE----------DEFINITIONS HERE

def plotLR(g, yL, yR, numGens, dest):
	# Plot the result using matplotlib
	fig = plt.figure(figsize=(20, 6))
	axL = fig.add_subplot(1,2,1)
	axL.scatter(g, yL, color='g', marker='o')
	axL.set_xlabel('Generation')
	axL.set_ylabel('Length')
	axL.set_title('Length over Generations (0-'+str(numGens)+')')

	axR = fig.add_subplot(1,2,2)
	axR.scatter(g, yR, color='g', marker='o')
	axR.set_xlabel('Generation')
	axR.set_ylabel('Radius')
	axR.set_title('Radius over Generations (0-'+str(numGens)+')')

	plt.savefig(dest+"/"+PlotName)
	plt.show()


#----------STARTS HERE----------STARTS HERE----------STARTS HERE----------STARTS HERE 


data = np.loadtxt(filePath + dataFile, skiprows=1)

print(data.shape)


'''
r = np.arange(0, 2, 0.01)
theta = 2 * np.pi * r

ax = plt.subplot(111, projection='polar')
ax.plot(theta, r)
ax.set_rmax(2)
ax.set_rticks([-3, -2, -1, 0, 1, 2])  # Less radial ticks
ax.set_rlabel_position(-22.5)  # Move radial labels away from plotted line
ax.grid(True)

ax.set_title("A line plot on a polar axis", va='bottom')
plt.show()

'''




