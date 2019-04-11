from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt
import csv

#u = np.array([1, 2, 3])
#v = np.array([4, 5, 6])

theta = []
phi=[]
gain=[]



with open('data.csv', 'r') as csvfile:
    plots= csv.reader(csvfile, delimiter=',')
    for i, row in enumerate(plots):
    	if i != 0:
	        theta.append(float(row[1]))
	        phi.append(float(row[2]))
	        gain.append(float(row[4]))
        
theta = np.array(theta)*np.pi/180.
phi = np.array(phi)*np.pi/180.
theta = theta[:325]
phi = phi[:325]
gain = gain[:325]

xcoord = gain*np.sin(theta)*np.cos(phi)
ycoord = gain*np.sin(theta)*np.sin(phi)
zcoord = gain*np.cos(theta)
'''
xMesh = np.ones((xcoord.shape[0], xcoord.shape[0]))*xcoord
yMesh = np.ones((xcoord.shape[0], xcoord.shape[0]))*ycoord

print(xMesh)
'''

print(xcoord[:5], ycoord[:5], zcoord[:5])

fig = plt.figure(figsize=(13,13))

ax_gainplot = fig.add_subplot(111, projection='3d')
ax_gainplot.scatter(xcoord, ycoord, zcoord)
ax_gainplot.set_title('Gain Plot')
ax_gainplot.set_xlabel('Theta')
ax_gainplot.set_ylabel('Phi')
ax_gainplot.set_zlabel('Gain')
plt.show()

"""
u=np.linspace(0, 2*np.pi, 100)
v=np.linspace(0, np.pi, 100)
r=10.

x = r*np.outer(np.cos(u), np.sin(v))
y = r*np.outer(np.sin(u), np.sin(v))
z = r*np.outer(np.ones(u.shape[0]), np.cos(v))
print(u, v)


fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

ax.plot_surface(x, y, z, color='blue')
plt.show()
"""