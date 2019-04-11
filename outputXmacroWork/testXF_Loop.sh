
rm output.xmacro

line1='var query = new ResultQuery();'
line2='///////////////////////Get Theta and Phi Gain///////////////'
line3='query.projectId = App.getActiveProject().getProjectDirectory();'
line4='query.simulationId = '
NPOP=10
FREQ=20

# Cat the relevant information onto output.xmacro for each antenna in a generation
zeroStr=('00000' '0000' '000' '00' '0')
#zeroInd=0

for ((freq=1; freq<=$FREQ; freq++))
do
	for ((indiv=1; indiv<=$NPOP; indiv++))
	do
		# Print the first 3 lines onto the .xmacro file
	    	echo "$line1" >> output.xmacro
	    	echo "$line2" >> output.xmacro
	    	echo "$line3" >> output.xmacro

		: 'To print the fourth line, we need to calculate the individual
		simulation ID. Remember that each antenna has $FREQ simulations/'
		indivFreqSimID=$(($indiv + $NPOP * ($freq - 1)))
		: 'We want the ID to be 6 digits, with zeros in front. 
		So, we choose the zero array element that, when pasted next 
		to the simulation ID, adds up to 6 digits'
		zeroInd=$((${#indivFreqSimID} - 1))
		
		echo "$line4\"${zeroStr[$zeroInd]}${indivFreqSimID}\""';'>> output.xmacro	    	
	    	cat outputmacroskeleton.txt >> output.xmacro
	done
done




