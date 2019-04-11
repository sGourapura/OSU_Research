
totGens=15
nPop=2
dest=Run6_4.11.19_nPop1_mean500_std250
gifName="MutMod10_nPop20_mean500_std250.gif"
gifFPS=4

# Delete any old generationDNA and fitnessScores files
rm generationDNA.csv
rm fitnessScores.csv

# Compile c++ file
g++ -std=c++11 roulette_algorithm.cpp
mv a.out roulette_alg.exe

# Create a runData file, if it isn't already there
mkdir runData

echo "Starting the first generation."
# Run the roulette algorithm for the first time
./roulette_alg.exe 'start' $nPop

# Move the generationDNA to save it
#cp generationDNA.csv runData/generationDNA0.csv

# Run the r_alg_tester
python3 r_alg_tester_gif.py 0

echo "Starting the subsequent generations"
for gen in `seq 1 $totGens`
do
	# Run the roulette algorithm for the consequent times
	./roulette_alg.exe 'cont' $nPop

	# Move the generationDNA to save it
	#cp generationDNA.csv runData/generationDNA${gen}.csv

	# Run the r_alg_tester
	python3 r_alg_tester_gif.py $gen
done


echo "Finished the Loop! Now prepping and saving the images..."


cd runData

python3 makeGif.py $totGens 1 $gifFPS

mkdir "$dest"
find -maxdepth 1 -name FScore\* -exec mv -t $dest {} +

mv myGifTest.gif "$dest/$gifName"

echo "Finished Everything!"
