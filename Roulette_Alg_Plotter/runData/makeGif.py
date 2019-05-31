

from PIL import Image
import argparse                 # For reading in arguments from terminalmerge_images(nameList)
import numpy as np
import moviepy.editor as mpy

# Generation number is taken by argparse from the user or bash script; we set this up first.
parser = argparse.ArgumentParser()
parser.add_argument("numGens", help="How many generations do you want to make into a gif?", type=int)
parser.add_argument("saveEvery", help="Which images did you want saved? Every 5 -> saveEvery = 5.", type=int)
parser.add_argument("FPS", help="What speed should the images be showed at? (e.g. 10)", type=int)
g = parser.parse_args()


def merge_images(nameList):
    """Merge two images into one, displayed side by side
    :param file1: path to first image file
    :param file2: path to second image file
    :return: the merged Image object
    """
    images = []
    for i in range(len(nameList)):
        images.append(Image.open(nameList[i]))
    '''
    (width1, height1) = image1.size
    (width2, height2) = image2.size
    
    result_width = width1 + width2
    result_height = max(height1, height2)
    '''
    width = images[0].size[0]
    height = images[0].size[1]
    result_width = width*len(nameList)
    result_height = height

    result = Image.new('RGB', (result_width, result_height))
    for i in range(len(nameList)):
        result.paste(im=images[i], box=(width*i, 0))
    return result


def make_gif(nameList):
    clip = mpy.ImageSequenceClip(nameList, fps=g.FPS)
    clip.write_gif('myGif.gif', fps=g.FPS)




numbs = np.arange(int(g.numGens/(g.saveEvery+0.)+1.))*g.saveEvery

nameList = []

for i in numbs:
    nameList.append("FScorePlot_gen"+str(int(i))+".png")

make_gif(nameList)
