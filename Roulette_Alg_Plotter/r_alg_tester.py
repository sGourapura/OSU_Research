# Written by: 	Suren Gourapura
# Written on: 	3/5/19
# Purpose: 	    Test the Roulette Algorithm and plot the results.
#               To change the fitness function, play with the variables sigma,
#               mu, and magnitude in FitnessFunction

import numpy as np              # For handling arrays
import matplotlib.pyplot as plt # For plotting and saving image
import argparse                 # For reading in arguments from terminal

#---------GLOBAL VARIABLES----------GLOBAL VARIABLES----------GLOBAL VARIABLES----------GLOBAL VARIABLES

gridDetail = 1000   # How fine of a grid do we use for the background fitness score plot?
contourDetail = 12  # How many contour lines do we want?
pad = 2.            # How far away should the borders of the plot be from the data?
dimension = 2       # Do we include angle data in the fitness score? (2 -> no, 3 -> yes)
saveEvery = 1       # Which plots do we save? Every 5th plot -> 5

# Generation number is taken by argparse from the user or bash script; we set this up first.
parser = argparse.ArgumentParser()
parser.add_argument("genNumb", help="Which generation is this run?", type=int)
g = parser.parse_args()

#----------DEFINITIONS HERE----------DEFINITIONS HERE----------DEFINITIONS HERE----------DEFINITIONS HERE

def Gaussian(indivs, mu, sigma, dim):
    # Displace x, y, and z from origin to mu
	x =indivs[0] - mu[0]
	y =indivs[1] - mu[1]
	z =indivs[2] - mu[2]
	# if we want a 2D gaussian, we force z = 0
	if dim == 2:
		z = 0
    # Calculate the radius
	r = np.sqrt( x**2 + y**2 + z**2 )
    # Calculate the value of the unit area gaussian
	fScore = 1/(sigma * np.sqrt(2*np.pi))
	fScore += np.exp(-(1/2) * (r/sigma)**2 )
	return fScore

def FitnessFunction(indivs):
	sigma1 = 1
	sigma2 = 4
	mu1 = [0, 10, 1]
	mu2 = [2, 20, 1]
	mag1 = 0
	mag2 = 20

	f_Func = Gaussian(indivs, mu1, sigma1, dimension)*mag1
	f_Func += Gaussian(indivs, mu2, sigma2, dimension)*mag2

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
	fig.colorbar(CP)

	# Add titles and labels
	ax.set_title("Fitness Function for Gen:"+str(g.genNumb))
	# Labels are x = radius, y = length, z = angle
	ax.set_xlabel("radius")
	ax.set_ylabel("length")
	# Return figure
	return fig, ax

def plotData(fig, ax, data, colors):
	ax.scatter(data[:,0], data[:,1], c=colors)
	
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
dataRange = findRange(data)


# CREATE PLOTS
# Create the background distribution
fig, ax = plotBackground(dataRange)

# Create the points
colors = 'red'
plotData(fig, ax, data, colors)

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





