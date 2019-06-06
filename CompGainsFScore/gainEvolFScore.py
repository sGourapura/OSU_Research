"""
Written by: 	Suren Gourapura
Date Written: 	06/06/2019
Goal:			Provide a fitness score that compares 2 gain patterns
Comments:		There will be a main fitness score function called 
				FScoreCalc. It will calculate the Chi Squared difference
				between 2 gain patterns.
"""
import numpy as np

# These are the paths to the test and target files
testFileName = 'data/testFile.uan'
targetFileName = 'data/targetFile.uan'

# Reads the fileName string's uan file
def readFile(fileName):
	
	uanData = np.loadtxt(fileName, delimiter=' ', skiprows=17)
	"""
	Now, we have all of the data in the file. However, we just need
	the first three columns (theta, phi, gain). Trim the data and 
	send it back.
	"""
	trimData = uanData[:,:3]
	# Calculate the discretization of the data 
	# (eg. deg increment = 5, 15, etc.)
	degIncrement = 90*(3 + np.sqrt(1 + 8*trimData.shape[0])) 
	degIncrement /= (trimData.shape[0] - 1)

	return trimData, int(degIncrement)

def FScoreCalc(testIndiv, targetIndiv):
	"""
	We grab 2 arrays, the target individual and the test individual.
	These are 325 x 3 if the data is in 15 degree increments.
	These are 2701 x 3 if the data is in 5 degree increments.

	To calculate the score, we simply calculate the chi squared 
	difference between our individual and the target individual.
	"""

	scores = np.zeros((testIndiv.shape[0]))

	# Now we calculate Chi Squared in a loop
	# This goes through every combination of theta and phi
	for i in range(testIndiv.shape[0]):
		# Calculate Chi Square
		scores[i] = ((testIndiv[i,2] - targetIndiv[i,2])**2) \
						/np.abs(targetIndiv[i,2])
	
	# Return the sum of the Chi Squares
	return np.sum(scores)

def Main():
	# Read the data
	testIndiv, testDegInc = readFile(testFileName)
	targetIndiv, targetDegInc= readFile(targetFileName)
	# Check to make sure the degree increments are the same
	if testDegInc != targetDegInc:
		print('Error: test and target uan files have different degree \
			increments.')

	# Calculate the score
	fScore = FScoreCalc(testIndiv, targetIndiv)

	return fScore


# Run Code
fScore = Main()
print(fScore)