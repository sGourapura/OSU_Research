# Written by: 	Suren Gourapura
# Written on: 	3/5/19
# Purpose: 	    Test the Roulette Algorithm and plot the results.
#               To change the fitness function, play with the variables sigma,
#               mu, and magnitude in FitnessFunction

import numpy as np              # For handling arrays
import matplotlib.pyplot as plt # For plotting and saving image
from matplotlib.animation import FuncAnimation
import argparse                 # For reading in arguments from terminal

#---------GLOBAL VARIABLES----------GLOBAL VARIABLES----------GLOBAL VARIABLES----------GLOBAL VARIABLES

gridDetail = 1000   # How fine of a grid do we use for the background fitness score plot?
contourDetail = 12  # How many contour lines do we want?
# Store xmin -> range[0,0], xmax -> range[0,1]
# Store ymin -> range[1,0], ymax -> range[1,1]
dataRange = np.array([[-1, 5], [0, 1000.]])
pad = 0.            # How far away should the borders of the plot be from the data?
dimension = 2       # Do we include angle data in the fitness score? (2 -> no, 3 -> yes)
saveEvery = 1       # Which plots do we save? Every 5th plot -> 5


# Generation number is taken by argparse from the user or bash script; we set this up first.
parser = argparse.ArgumentParser()
parser.add_argument("genNumb", help="Which generation is this run?", type=int)
g = parser.parse_args()

#----------DEFINITIONS HERE----------DEFINITIONS HERE----------DEFINITIONS HERE----------DEFINITIONS HERE

def Gaussian(indivs, mu, sigma, sigmaRatio, dim):
    # Displace x, y, and z from origin to mu
	x =indivs[0] - mu[0]
	y =indivs[1] - mu[1]
	z =indivs[2] - mu[2]
	# if we want a 2D gaussian, we force z = 0
	if dim == 2:
		z = 0
    # Calculate the radius
	r = np.sqrt( x**2 + (y/sigmaRatio)**2 + z**2 )
    # Calculate the value of the unit area gaussian
	fScore = 1/(sigma * np.sqrt(2*np.pi))
	fScore += np.exp(-(1/2) * (r/sigma)**2 )
	return fScore

def FitnessFunction(indivs):
	sigma1 = 1
	sigma2 = 1
	sigmaRatio1 = 100*0.8
	sigmaRatio2 = 100*0.8
	mu1 = [2, 750, 1]
	mu2 = [2, 250, 1]
	mag1 = 20
	mag2 = 10

	f_Func = Gaussian(indivs, mu1, sigma1, sigmaRatio1, dimension)*mag1
	f_Func += Gaussian(indivs, mu2, sigma2, sigmaRatio2, dimension)*mag2

	return f_Func

def plotBackground(rng):
	# Create a linear grid of points to plot the background.
	# Use data range to determine plot range. pad adds a bit extra
	x_pts = np.linspace(rng[0,0]-pad, rng[0,1]+pad, gridDetail)
	y_pts = np.linspace(rng[1,0]-pad, rng[1,1]+pad, gridDetail)
	x, y = np.meshgrid(x_pts, y_pts)
	vec = np.array([x, y, np.zeros((x.shape))])

	# Calculate the background distribution values
	z = FitnessFunction(vec)

	# plot background distribution
	fig = plt.figure(figsize=(10, 10))
	ax = fig.add_subplot(1,1,1)

	CP = ax.contourf(x, y, z, contourDetail)
	ax.contour(CP, colors='k')
	cbar = fig.colorbar(CP)
	cbar.set_label('Fitness Score', size=18)
	cbar.ax.tick_params(labelsize=18)

	# Add titles and labels
	ax.set_title("Fitness Function for Gen:"+str(g.genNumb), fontsize=22)
	# Labels are x = radius, y = length, z = angle
	ax.set_xlabel("radius", fontsize=18)
	ax.set_ylabel("length", fontsize=18)
	ax.xaxis.set_tick_params(labelsize=20)
	ax.yaxis.set_tick_params(labelsize=20)
	# Return figure
	return fig, ax

def plotData(fig, ax, data, colors, rng):
	ax.scatter(data[:,0], data[:,1], c=colors)
	ax.set_xlim(rng[0])
	ax.set_ylim(rng[1])
def findRange(data):
	rng = np.zeros((2,2))
	# Store xmin -> range[0,0], xmax -> range[0,1]
	rng[0,0] = data[:,0].min()
	rng[0,1] = data[:,0].max()
	# Store ymin -> range[1,0], ymax -> range[1,1]
	rng[1,0] = data[:,1].min()
	rng[1,1] = data[:,1].max()
	return rng
#----------STARTS HERE----------STARTS HERE----------STARTS HERE----------STARTS HERE 


# LOAD DATA
print('\nStarting calculations and plot for generation '+str(g.genNumb)+'.\n')
# data is (NPOP x 3)
data = np.loadtxt("generationDNA.csv", delimiter=',', skiprows=9)
#print('generationDNA matrix: \n')
#print(data, "\n")

# Calculate the fitness scores of the data
fScores = FitnessFunction(data.T)
print('Top five fitness scores')
# This bit below is somewhat complicated! np.sort sorts from least to greatest,
# but we want greatest to least. So, sort the negative version, then make it positive
# again. Finally, show just the top 5
print(-np.sort(-fScores)[:5], "\n")

# Find the minimum and maximum values for x and y.
#dataRange = findRange(data)


# CREATE PLOTS
# Create the background distribution
fig, ax = plotBackground(dataRange)

# Create the points
colors = 'red'
plotData(fig, ax, data, colors, dataRange)

# Save figure and plot on screen
if g.genNumb%saveEvery ==0:
	fig.savefig('runData/FScorePlot_gen'+str(g.genNumb)+'.png')
#plt.show()


# WRITE NEW FITNESS SCORE
# Write new fitnessScores.csv file
head1 = "The Ohio State University GENETIS Data."
head2 = "Current generation's fitness scores:"
with open("fitnessScores.csv", "w") as fScoreFile:
	fScoreFile.write(head1 +'\n'+head2 +'\n')
	np.savetxt(fScoreFile, fScores, fmt="%0.4f")

print('Finished generation '+str(g.genNumb)+'.\n')





