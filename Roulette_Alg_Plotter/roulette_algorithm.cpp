  
/*	This GA is adapted from CalPoly's hybrid roulette / tournament method to work with Ohio State's loop. 
	Written by David Liu
	Revised by Suren Gourapura to accept NPoP on 29 Dec 2018
	
	Everything is carefully commented, so things should hopefully be clear.
	
*/

// Compile using: g++ -std=c++11 roulette_algorithm.cpp roulette_alg.exe
#include <time.h>
#include <math.h>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <math.h>

using namespace std;

// Headers for subroutines we'll call in main

/*	Inputs for function dataRead: 
	varInput = tensor for variables to be read by the program (parent population)
	fitness = vector storing the fitness scores for each parent
*/
void dataRead(vector<vector<vector<float>>> &varInput, vector<float> &fitness);

/*	Inputs for function dataWrite: 
	numChildren = total number of children (population size)
	varVector = variable data for the children to be written to generationDNA
	freq_coeffs = number of frequencies we're running with (named coeffs since later we'll be using pulses with coefficients)
	freqVector = all the frequency coefficients are stored in this vector
*/
void dataWrite(int numChildren, vector<vector<vector<float>>> &varVector, int freq_coeffs, vector<double> freqVector);

/* 	Inputs for function checkConvergence:
	varInput = variables we read from previous generation
	fitness = previous generation's fitness scores
*/
int checkConvergence(vector<vector<vector<float>>> &varInput, vector<float> &fitness);

/*	Inputs for function roulette:
	varInput = variables we read from previous generation
	varOutput = variables we generate for next generation
	parents = tensor to store the variables used to generate an individual
	fitness = fitness scores
*/
void roulette(vector<vector<vector<float>>> &varInput, vector<vector<vector<float>>> &varOutput, vector<float> &fitness);

/* 	Inputs for function tournament:
	varInput = variables we read from previous generation
	varOutput = variables we generate for next generation
	parents = tensor to store the variables used to generate an individual
	fitness = fitness scores
*/

void tournament(vector<vector<vector<float>>> &varInput, vector<vector<vector<float>>> &varOutput, vector<float> &fitness);

// Declare some global constants

/*
	Nomenclature:
	I refer to the data lines and data structure as "chromosomes" and "genes".
	
	DATA STRUCTURE:
	
	                        GENE 1           GENE 2           ...           GENE NVARS
	CHROMOSOME 1           VARIABLE         VARIABLE        VARIABLE         VARIABLE
	
	CHROMOSOME 2           VARIABLE         VARIABLE        VARIABLE         VARIABLE
	
	...
	
	CHROMOSOME NSECTIONS   VARIABLE         VARIABLE        VARIABLE         VARIABLE
	
	Each "chromosome" will hold the data for a particular section of the antenna (if we try to form paperclip antennae later,
	I envision this being useful for example). Each "gene" represents a particular type of variable; for example,
	Gene 1 might represent length. Then gene 1 on EVERY chromosome should represent the length of that section.
	Then in the resulting matrix, (0,0) (1,0) (2,0) ... all encode for the length of section 0, section 1, section 2, etc.
	This should theoretically provide some sort of order and stability across the board. This also generalizes this program
	to be able to work with an arbitrary number of sections or variables.
*/

// SIMULATION CONSTANTS

const double MINIMUM_FREQUENCY = 0.08333; // This global constant defines the minimum frequency of the range we're working in, in GHz

const double MAXIMUM_FREQUENCY = 1.0667; // This global constant defines the maximum frequency of the range we're working in, in GHz

const double FREQ_STEP = 0.01667; // This global constant defines the step size between frequencies as we scan.

//	DNA RELATED CONSTANTS

const int NSECTIONS = 1;  // This global constant controls the number of chromosomes we are working with. This is equal to the number of sections in the antenna.

const int NVARS = 3; // This global constant controls the number of genes we are working with. This is equal to the number of variables per section (e.g. length, radius, angle, coordinates, etc)

const int PARENT_NO = 2; // This global constant controls the number of potential parents per child. If 1, each child will be a clone of the parent.

const int DNA_GARBAGE_END = 9; // This global constant defines where in generationDNA.csv data begins. Data should begin on the NEXT line after this; so if 8, the first line of data is on line 9.

// ALGORITHM RELATED CONSTANTS

int NPOP; /* This global constant represents how many individuals there are per generation. It's value is determined by the user, in argv[2]. It cannot be cast as a constant, because the user's value of NPOP has to be defined inside int main. BE VERY CAREFUL NOT TO REDEFINE NPOP SOMEWHERE IN THE CODE! */

const float MUTABILITY  = 0.6f; // This global constant defines overall mutability. Every generation will have this percentage of their offspring mutated.

/* TOURNAMENT RELATED CONSTANTS
	HOW TOURNAMENT WORKS:
	
	We randomly select a sample of TOURNEY_LOTTERY_SIZE individuals from the parent population. We then select the individual from that lottery who has the HIGHEST fitness score.
	That individual becomes a parent.
	
	The other individuals are returned to the pool. Then we randomly select a new sample of TOURNEY_LOTTERY_SIZE individuals from the new pool, and repeat the process until we have enough
	parents.
	
	Then, we return ALL individuals to the pool, regardless of if they were chosen as parents or not and we begin picking new parents for a NEW offspring. So the process is
	
	INDIVIDUAL X:
	
	
	------------------------------
	| POOL OF POTENTIAL PARENTS |   <-----------
	------------------------------             |
	|                                          |
	|                                          |
	| RANDOM DRAWING OF INDIVIDUALS            |
	V                                          |
	                                           |
	---------------------                      |
	| RANDOM SELECTION |                       | RETURN ALL NON-SELECTEES BACK TO THE POTENTIAL POOL
	---------------------                      | FOR ANOTHER CHANCE TO BE SELECTED AND BECOME PARENTS
	|                                          | 
	|                                          |
	| WHO HAS THE HIGHEST FITNESS SCORE? ------|
	V                                          
	                                           
	------------------                         
	| RANDOM PARENT |   -------> THIS PARENT BECOMES A PARENT FOR INDIVIDUAL 1.
	------------------
	
	
	
	MODIFYING TOURNAMENT:
	To change the number of potential parents, change TOURNEY_LOTTERY_SIZE.
	
	To change the number of offspring we generate, change TOURNEY_PROPORTION.
	
	MUTABILITY defines how many mutations we introduce per generation. If we introduce a lot, we'll have much quicker
	changes in our fitness scores, but we might take a while to actually converge.
*/

const float TOURNEY_PROPORTION = 0.0f; // This value represents what percentage of the children generated use tournament-style selection. Must be 0 < x < 1.

const int TOURNEY_LOTTERY_SIZE = 4; // The number of potential parents selected every time we pick a parent.

// STATISTICS RELATED CONSTANTS

const float CONVERGENCE = 0.00; // Our convergence threshold. If the standard deviation of the last generation is within this value, we terminate the loop and return data.

// Gene One, Gene One controls radius currently

const float INITIAL_MEAN_C1_G1 = 2.0f;//1.0f; // When we initially run, this is the average value seeded for the gene.

const float INITIAL_STD_DVN_C1_G1 = 0.5f; //0.25f; // When starting a new generation, this is the starting variance for the gene.

// Gene Two, Gene One controls length currently

const float INITIAL_MEAN_C1_G2 = 500.0f;//15.0f; // When we initially run, this is the average value seeded for the gene.

const float INITIAL_STD_DVN_C1_G2 = 250.0f;//10.0f; // When starting a new generation, this is the starting variance for the gene.

// Gene Three, Gene One controls angle (in radians) currently

const float INITIAL_MEAN_C1_G3 = M_PI / 4;//8; // When we initially run, this is the average value seeded for the gene.

const float INITIAL_STD_DVN_C1_G3 = M_PI / 4;//8; // When starting a new generation, this is the starting variance for the gene.

// All other genes are currently unused
const float INITIAL_MEAN_CX_GY = 0.0f;

const float INITIAL_STD_DVN_CX_GY = 0.0f;

const float MUT_MODULATOR = 10.0f; // This is inversely proportional to the magnitude of mutations. At 2, all mutations are halved in magnitude. The higher this goes, the slower evolution becomes.

int main(int argc, char const *argv[])
{
    /*
	argc = number of console commands
	argv[] = actual console commands 
	varInput = tensor for variables read by the program (parents)
	varOutput = tensor for variables written by the program (children)
	fitness = vector for fitness scores read by the program
    */
	
	/* Create vector for parsing in variables. We're only working with length and radius at the moment, so this is only 1x2.
	// But later on, we'll be working with many more variables, so this should theoretically save some time when we do by writing this way.
	
	// Vector varInput is an  n x NSECTIONS x NVARS tensor, where n is the number of individuals in the population. Basically, every matrix stores the data for each individual.
	// Honestly we probably don't need a matrix for all the variables, but we MIGHT need that much space down the line, so might as well. Data structure is:
	
	INDIVIDUAL 1
	                 GENE 1     GENE 2    GENE 3     GENE 4      ...     GENE Z
	CHROMOSOME 1    RADIUS 1   LENGTH 1    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 2	RADIUS 2   LENGTH 2    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 3	RADIUS 3   LENGTH 3    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 4	RADIUS 4   LENGTH 4    VAR 3      VAR 4      ...      VAR Z
	... 
	CHROMOSOME Y	RADIUS Y   LENGTH Y    VAR 3       VAR 4     ...      VAR Z
	INDIVIDUAL 2
	                 GENE 1     GENE 2    GENE 3     GENE 4      ...     GENE Z
	CHROMOSOME 1    RADIUS 1   LENGTH 1    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 2	RADIUS 2   LENGTH 2    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 3	RADIUS 3   LENGTH 3    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 4	RADIUS 4   LENGTH 4    VAR 3      VAR 4      ...      VAR Z
	... 
	CHROMOSOME Y	RADIUS Y   LENGTH Y    VAR 3       VAR 4     ...      VAR Z
	
	...
	
	INDIVIDUAL X
	                 GENE 1     GENE 2    GENE 3     GENE 4      ...     GENE Z
	CHROMOSOME 1    RADIUS 1   LENGTH 1    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 2	RADIUS 2   LENGTH 2    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 3	RADIUS 3   LENGTH 3    VAR 3      VAR 4      ...      VAR Z
	CHROMOSOME 4	RADIUS 4   LENGTH 4    VAR 3      VAR 4      ...      VAR Z
	... 
	CHROMOSOME Y	RADIUS Y   LENGTH Y    VAR 3       VAR 4     ...      VAR Z
	
	*/

	// First, define NPOP using the user's input. The atoi function converts from string to int
	NPOP = atoi(argv[2]);

    	vector<vector<vector<float>>> varInput (NPOP,vector<vector<float> >(NSECTIONS,vector <float>(NVARS, 0.0f)));
	
	// Vector fitness literally just exists to tell us the fitness scores later
	
    	vector<float> fitness (NPOP, 0.0f);
	
	// Create vector that will hold the children's variables. Same structure as varInput.
	
    	vector<vector<vector<float>>> varOutput (NPOP,vector<vector<float> >(NSECTIONS,vector <float>(NVARS, 0.0f)));
	
	int freq_coeffs = round((MAXIMUM_FREQUENCY - MINIMUM_FREQUENCY) / FREQ_STEP + 1);
	
	vector<double> freqVector (freq_coeffs, 0.0);
	
	// We'll just calculate and input our frequencies now - they aren't used anywhere else in the program.
	
	freqVector[0] = MINIMUM_FREQUENCY;
	
	for(int i=1; i<freq_coeffs; i++)
	{
		freqVector[i] = MINIMUM_FREQUENCY + (FREQ_STEP * i);
	}
	
	srand((unsigned)time(0)); // Let's just seed our random number generator off the bat
	
    	// Read in input arguments and parse in data from files
	
	cout << "Roulette algorithm initialized." << endl;
	
    	if(argc != 3)
        {cout << "Error: Usage. Specify start or cont, as well as NPOP (EX: start 10)." << endl;}
    	else
    	{
      		if(string(argv[1]) == "start")
		{
			/*
				These loops should generate a random number from a normalized distribution with mean 0.5 and sigma 0.25
				to populate every entry in our matrix that we're using. Currently, that's only a 1x2 vector, but later
				it could be a lot bigger.
				Sample:
			
				Current:
			
				r r 0 0 0 ... 0
				0 0 0 0 0 ... 0
				.             .
				.             .
				.             .
				0 0 0 0 0 ... 0
			
				My thought process currently is each row represents sections of the antenna (to accommodate weird shapes)
				and then each column represents a specific variable in that section (currently just length and radius)
			*/

			std::default_random_engine generator;
			generator.seed(time(0));
			std::normal_distribution <float> distribution_radius(INITIAL_MEAN_C1_G1, INITIAL_STD_DVN_C1_G1);
			std::normal_distribution <float> distribution_length(INITIAL_MEAN_C1_G2, INITIAL_STD_DVN_C1_G2);
			std::normal_distribution <float> distribution_angle(INITIAL_MEAN_C1_G3, INITIAL_STD_DVN_C1_G3);
		
			for(int i=0;i<NPOP;i++)
			{
				for(int j=0;j<NSECTIONS;j++)
				{
					for(int k=0;k<NVARS;k++)
					{
						if (k == 0)
						{
						float r = distribution_radius(generator);
					
						while(r<=0) // We don't accept negative or zero values
							r = distribution_radius(generator);
					
						varOutput[i][j][k]= r;
						}
					
						else if (k == 1)
						{
						float r = distribution_length(generator);
					
						while(r<=0) // We don't accept negative or zero values
							r = distribution_length(generator);
					
						varOutput[i][j][k]= r;
						}	

						else if (k == 2)
						{
						float r = distribution_angle(generator);
					
						while(r<0) // We don't accept negative values
							r = distribution_angle(generator);
					
						varOutput[i][j][k]= r;
						}	
					}
				}
			}
			
			/* For the time being we comment this out. Need to figure out how to make every gene
			 * counted with a different starting seed later.
		
			std::default_random_engine generator;
			generator.seed(time(0));
			std::normal_distribution <float> distribution(INITIAL_MEAN, INITIAL_STD_DVN);
		
			for(int i=0;i<NPOP;i++)
			{
				for(int j=0;j<NSECTIONS;j++)
				{
					for(int k=0;k<NVARS;k++)
					{
						float r = distribution(generator);
					
						while(r<=0) // We don't accept negative or zero values
							r = distribution(generator);
					
						varOutput[i][j][k]= r;
					}
				}
			}
			*/
		
			// Next up we write to file generationDNA
			dataWrite(NPOP, varOutput, freq_coeffs, freqVector);
			double meanTotal = 0.0;
			for(int i=0; i<NPOP; i++)
				{
					meanTotal = meanTotal + varOutput[i][0][1];
				}
			float meanForGridSize = meanTotal / NPOP;
			ofstream datasize;
			datasize.open("datasize.txt");
			datasize << meanForGridSize/50.0 << ";";
			datasize.close();	
		}
		else if(string(argv[1]) == "cont")
		{
			  dataRead(varInput,fitness); // Read in the stuff from previous generation
			  if (checkConvergence(varInput,fitness) == 1) // We check for convergence. If we've converged then end loop
			  {
				  remove("highfive.txt"); // we delete the old highfive.txt that has a 0 in it
				  ofstream highfive;
				  highfive.open("highfive.txt"); // we create a new highfive.txt that will have a 1 in it
				  highfive << 1;
				  highfive.close();
			  }
			  else // If no convergence, generate a new generation and write a new generationDNA.csv
			  {
				  roulette(varInput,varOutput,fitness);
				  cout << "Roulette complete." << endl;
				  tournament(varInput,varOutput,fitness);
				  cout << "Tournament complete." << endl;
				  dataWrite(NPOP, varOutput, freq_coeffs, freqVector);
				  double meanTotal = 0.0;
				  for(int i=0; i<NPOP; i++)
				  {
					  meanTotal = meanTotal + varOutput[i][0][1];
				  }
				  float meanForGridSize = meanTotal / NPOP;
				  ofstream datasize;
				  datasize.open("datasize.txt");
				  datasize << meanForGridSize/50.0 << ";";
				  datasize.close();				  
			  }
		}
	}
	return (0);
}

	/*		
		As currently written function dataWrite outputs an NSECTIONS x NVARS sized matrix. 
		NSECTIONS represents how many segments in our antenna we're generating for.
		Currently this is only 1 since we're just working with evolving a dipole.
		NVARS represents how many variables in that section we're working with.
		Currently this is only 2 since we're just working with length and radius.
		
		Currently, this function should output something like:
		
		Hybrid of Roulette and Tournament -- Thanks to Cal Poly, Jordan Potter
		Author was David Liu
		Notable Contributors: Julie, Hannah Hasan, and Adam Blenk
		Done at The Ohio State University
		Working on behalf of Dr. Amy Connolly
		And the ANITA project
		Revision date: 21 March 2018 1800 EST
		Matrices for this Generation: 
		R1,L1
		R2,L2
		...
		RX,LX
		
		In the future, it'll output something like:
		
		Hybrid of Roulette and Tournament -- Thanks to Cal Poly, Jordan Potter
		Author was David Liu
		Notable Contributors: Julie, Hannah Hasan, and Adam Blenk
		Done at The Ohio State University
		Working on behalf of Dr. Amy Connolly
		And the ANITA project
		Revision date: 21 March 2018 1800 EST
		Matrices for this Generation: 
		R1,L1,VARA,VARB...VARZ
		R2,L2,VARA,VARB...VARZ
		.                 .
		.                 .
		.                 .
		R99,L99,VARX,VARY...VARZ
		R1,L1,VARA,VARB...VARZ
		R2,L2,VARA,VARB...VARZ
		.                 .
		.                 .
		.                 .
		R99,L99,VARX,VARY...VARZ
		...
		R1,L1,VARA,VARB...VARZ
		R2,L2,VARA,VARB...VARZ
		.                 .
		.                 .
		.                 .
		R99,L99,VARX,VARY...VARZ
	*/
	
void dataWrite(int numChildren, vector<vector<vector<float>>> &varVector, int freq_coeffs, vector<double> freqVector)
{
	ofstream generationDNA;
	generationDNA.open("generationDNA.csv");
	generationDNA << "Hybrid of Roulette and Tournament -- Thanks to Cal Poly / Jordan Potter" << "\n";
	generationDNA << "Author was David Liu" << "\n";
	generationDNA << "Notable contributors: Julie Rolla, Hannah Hasan, and Adam Blenk" << "\n";
	generationDNA << "Done at The Ohio State University" << "\n";
	generationDNA << "Working on behalf of Dr. Amy Connolly" << "\n";
	generationDNA << "And the ANITA project" << "\n";
	generationDNA << "Revision date: 21 March 2018 1800 EST" << "\n";
	for(int i=0;i<freq_coeffs;i++)
	{
		if(i==freq_coeffs-1)
		{
			generationDNA << freqVector[i] << "\n";
		}
		else
		{
			generationDNA << freqVector[i] << ",";
		}
	}
	generationDNA << "Matrices for this Generation: " << "\n";
	for(int i=0;i<numChildren;i++)
		{
			for(int j=0;j<NSECTIONS;j++)
			{
				for(int k=0;k<NVARS;k++)
				{
					if(k==(NVARS-1))
					{
						generationDNA << varVector[i][j][k] << "\n";
					}
					else
					{
						generationDNA << varVector[i][j][k] << ",";
					}
				}
			}
		}
	generationDNA.close();
}

void dataRead(vector<vector<vector<float>>> &varInput, vector<float> &fitness)
{
	ifstream generationDNA;
	generationDNA.open("generationDNA.csv");
	int csv_file_size = DNA_GARBAGE_END + (NPOP * NSECTIONS);
	string csvContent[csv_file_size+1]; //contain each line of csv
	string strToDbl; //data change from string to float, then written to varInput or fitness.
	
	/*
		This loop reads through the .csv file line by line.
		If we're in data (past line 9), it reads in the line.
	*/
	for(int i=1;i<=csv_file_size;i++)
	{
		getline(generationDNA,csvContent[i]);
		if (i>DNA_GARBAGE_END)
		{
			double j=floor((i-DNA_GARBAGE_END)/NSECTIONS);  // Figure out which individual we're looking at
			int p=i-DNA_GARBAGE_END-NSECTIONS*j; // pulls out which row of their matrix we're looking at 
			istringstream stream(csvContent[i]);
			for(int k=0;k<NVARS;k++)
				{
					getline(stream,strToDbl,',');
					varInput[j-1][p][k] = atof(strToDbl.c_str());
					//cout << "Individual " << j << " has data points " << varInput[j-1][p][k] << endl;
				}
		}
	}
	
	generationDNA.close();
	
	// Now we need to read the fitness scores:
	ifstream fitnessScores;
	fitnessScores.open("fitnessScores.csv");
	string fitnessRead[NPOP+2];
	for(int i=0;i<(NPOP+2);i++)
    {
		getline(fitnessScores,fitnessRead[i]);
		if(i>=2)
		{
			fitness[i-2] = atof(fitnessRead[i].c_str());
			if (fitness[i-2]<0)
			{
				fitness[i-2] = 0; // If the fitness score is less than 0, we set it to 0 to not throw things off.
			}
		}
    }
	fitnessScores.close();
}


int checkConvergence(vector<vector<vector<float>>> &varInput, vector<float> &fitness)
{
	/* 	
		To test convergence we look at standard deviation. To calculate it for each variable,
		we need the mean first; then we calculate sigma. I'm writing them as tensors for future
		because currently we only test the length for convergence, but down the line we may need
		to test every variable.
	*/
	
	vector<vector<float>> meanTensor (NSECTIONS,vector<float>(NVARS,0));
	vector<vector<float>> dvnTensor (NSECTIONS,vector<float>(NVARS,0));
	
	for (int i=0;i<NVARS;i++)
	{
		for (int j=0;j<NSECTIONS;j++)
		{
			float totalSum=0.0f;
			for (int k=0;k<NPOP;k++)
			{
				totalSum+=varInput[k][j][i];
			}
			float mean = totalSum / NPOP;
			meanTensor[j][i]=mean;
		}
	}
	
	for (int i=0;i<NVARS;i++)
	{
		for (int j=0;j<NSECTIONS;j++)
		{
			float dvnSum=0.0f;
			for (int k=0;k<NPOP;k++)
			{
				dvnSum+=pow((varInput[k][j][i]-meanTensor[j][i]),2);
			}
			float dvn = pow((dvnSum / (NPOP - 1)),1/2);
			dvnTensor[j][i]=dvn;
		}
	}
	
	if (dvnTensor[0][1] <= CONVERGENCE) // This is hella specific and will need to be fixed later.
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void roulette(vector<vector<vector<float>>> &varInput, vector<vector<vector<float>>> &varOutput, vector<float> &fitness)
{
	/* Roulette selection works as follows:
		Every parent in the population has a chance of having their genes selected. Each gene is selected by the probability of:
		
			     fitness
		p = -------------------
		    sum of all fitness
		
		Which is to say higher fitness score parents are more likely to have their genes selected, but all genes have a chance of
		being expressed. We use a partial sum to select to guarantee that at least one gene will be selected by the end.
		So currently as written we do it this way:
		
		p1 = f1 / F
		p2 = (f1 + f2) / F
		p3 = (f1 + f2 + f3) / F
		...
		p10 = F / F
		
		We then create roul_no * MUTABILITY mutant offspring; which is to say we run through and randomly change genes by a small amount.
		I currently am selecting two parents through roulette wheel selection for every 
	*/
	
	float fitness_total=0;
	int roulette_no=NPOP*(1-TOURNEY_PROPORTION);
	int tourney_no=NPOP*TOURNEY_PROPORTION;
	
	// Sanity check to make sure we have NPOP offspring from using proportions. Just in case someone does something weird with like 47 individuals
	// Per generation and a tourney proportion of like 31%.
	
	while (roulette_no+tourney_no!=NPOP)
	{
		if (roulette_no+tourney_no>NPOP && roulette_no>tourney_no)
		{
			roulette_no+=-1;
		}
		else if (roulette_no+tourney_no>NPOP && roulette_no<tourney_no)
		{
			tourney_no+=-1;
		}
		else if (roulette_no+tourney_no<NPOP && roulette_no>tourney_no)
		{
			roulette_no+=1;
		}
		else if (roulette_no+tourney_no<NPOP && roulette_no<tourney_no)
		{
			tourney_no+=1;
		}
	}
			
	vector<vector<int>> selected (roulette_no,vector<int>(PARENT_NO,0));
	
	// Calculate total fitness for our probabilities.
	for(int i=0;i<NPOP;i++)

	{
		fitness_total+=fitness[i];
	}
	
	/** Roulette Wheel Selection **/
	/*
		How these loops work:
		i loop runs through and makes the entire loop run for each individual offspring
		in other words, the i loop generates parents for the ith offpsring
		
		j loop runs through and runs the parent selection loop until we have enough parents for each offspring
		in other words, the j loop generates j parents for the ith offspring
		
		k loop runs through and actually selects parents. It does this by taking the partial sum of each successive parent in
		the fitness vector. Then it looks how what percentage of the total sum the partial sum is, and comparing that percentage to
		the random number 1-100 generated earlier. If the partial sum is greater than the random number previously generated,
		it records the location of which parent's fitness score was added to exceed the partial sum barrier
		(i.e. if it had to add 7 partial sums together, it records 6, since vectors record locations 0-9, so the 7th parent is at location [6])
		It then breaks the loop and returns to the j loop
		
		The j loop then records the parent's location in a matrix. Each row in our matrix represents a particular offspring,
		and each column represents parents. So the data structure for the selected matrix is as follows:
		
		              PARENT LOCATION    PARENT LOCATION    PARENT LOCATION    ...    PARENT LOCATION    
		OFFSPRING 1        count              count              count                     count
		OFFSPRING 2        count              count              count                     count
		OFFSPRING 3        count              count              count                     count
		...                ...                ...                ...                       ...
		OFFSPRING X        count              count              count                     count
		
		Incidentally, this matrix is of size roulette_no x PARENT_NO, which is currently 5x2.
		But if we increase the number of offspring, we increase rows
		And if we increase the number of parents, we increase columns.
	*/
	for(int i=0;i<roulette_no;i++)  // run for however many kids we are generating
	{
		for(int j=0;j<PARENT_NO;j++)  // run for however many parents per kid
		{
			float partial_sum=0.0f; // reset the partial sum to 0 every time we look for a new parent
			float r=rand()%100; // we randomly select a new chance that must be met every time we find a parent
			
			for(int k=0;k<NPOP;k++) // running through the possible parents (10)
			{
				partial_sum = partial_sum + fitness[k];
				if(r<(100*partial_sum/fitness_total)) // if our randomly selected chance is less than the partial sum / total sum * 100, we have a parent
				{
					selected[i][j]=k; // assign parent number j for kid number i
					break;
				}
			}
		}
	}

	// Now we actually use the parents and begin breeding offpsring
	// These are just crossbreeds without any mutations yet.
	/* We use uniform crossover rather than single or double point selection. What this means is:
	
	When we do crossover, we randomly select genes from either parent. So each gene has a 50% chance
	from each parent to be selected. Because every gene can be exchanged randomly, instead of like in
	single / double point selection where long strands of chromosomes are exchanged, we search the entire
	solution space much more quickly and thoroughly than we would with traditional genetic crossover.
	
	Incidentally, we also don't do what most GAs do with generating two children per two parents (example here):
	Child 1: P1G1 P2G2 P2G3 P2G4 P1G5 P1G6
	Child 2: P2G1 P1G1 P1G3 P1G4 P2G5 P2G6
	
	Where each child has the gene from the opposite parent as her sibling, to increase genetic diversity
	and give each parent from the previous generation more of a possibility of being a parent. This further
	improves the thoroughness of the solution space search.
	*/
	
	for(int i=0;i<roulette_no;i++)
	{
		for(int j=0;j<NSECTIONS;j++)
		{
			for(int k=0;k<NVARS;k++)
			{
				int pick1=rand()%PARENT_NO; // generates a random integer number between 0 and number of parents
				//cout << "Parent " << pick1 << " selected with value " << varInput[selected[i][pick1]][j][k] << endl;
				varOutput[i][j][k]=varInput[selected[i][pick1]][j][k]; // This looks hella messy, but really all it's saying is go to randomly chosen parent and give the offspring the allele there.
			}
		}
	}
	
	// Now we have roulette_no offspring, but we're not done yet! We need to mutate them. To do that we first need to know
	// the average and the deviation of the parent generation. So let's go ahead and calculate that here:
	
	vector<vector<float>> meanTensor (NSECTIONS,vector<float>(NVARS,0));
	vector<vector<float>> dvnTensor (NSECTIONS,vector<float>(NVARS,0));
	
	for (int i=0;i<NVARS;i++)
	{
		for (int j=0;j<NSECTIONS;j++)
		{
			float totalSum=0.0f;
			for (int k=0;k<NPOP;k++)
			{
				totalSum+=varInput[k][j][i];
			}
			float mean = totalSum / NPOP;
			meanTensor[j][i]=mean;
		}
	}
	
	for (int i=0;i<NVARS;i++)
	{
		for (int j=0;j<NSECTIONS;j++)
		{
			float dvnSum=0.0f;
			for (int k=0;k<NPOP;k++)
			{
				dvnSum+=pow((varInput[k][j][i]-meanTensor[j][i]),2);
			}
			float dvn = pow((dvnSum / (NPOP - 1)),1/2);
			dvnTensor[j][i]=dvn;
		}
	}
	
	// Now we can do the mutations!
	vector<bool> mutate_flag (roulette_no,false); // Stores if a kid has already been exposed to mutagens. No need to mutate them further
	
	// Calculate how many mutants we need to generate
	
	int roul_mut = roulette_no * MUTABILITY;
	
	for(int i=0;i<roul_mut;i++)
	{ 
		int r = rand()%roulette_no; // Let's pick a random kid to expose to mutagens
		
		while(mutate_flag[r] == true) // If we already mutated that kid, pick again
		{
			r = rand()%roulette_no;
		}
		for(int j=0;j<NSECTIONS;j++)
		{
			int numberOfMutations = rand()%NVARS + 1; // We apply anywhere from 1 - NVARS mutations)
			
			for(int k=0; k < numberOfMutations; k++)
			{
				/* This section determines the magnitude of the mutation we apply and which gene to mutate */
				int chromosomeMutation = rand()%NSECTIONS; // We randomly select which chromosome to mutate
				int geneMutation = rand()%NVARS; // We randomly select which gene to mutate
				std::default_random_engine generator;
				generator.seed(time(0));
				std::normal_distribution <float> distribution(meanTensor[chromosomeMutation][geneMutation],dvnTensor[chromosomeMutation][geneMutation]);
				
				/* This section determines whether or not the mutation adds or subtracts, and actually applies it */
				int coefficient=rand()%2;
				if(coefficient==0)
				{
					varOutput[r][chromosomeMutation][geneMutation]=varOutput[r][chromosomeMutation][geneMutation]+(distribution(generator)/MUT_MODULATOR);  // divides by MUT_MODULATOR to modulate the effect so we don't get big mutations.
				}
				else
				{
					varOutput[r][chromosomeMutation][geneMutation]=varOutput[r][chromosomeMutation][geneMutation]-(distribution(generator)/MUT_MODULATOR);  // divides by MUT_MODULATOR to modulate the effect so we don't get big mutations.
				}				
				while(varOutput[r][chromosomeMutation][geneMutation]<=0) // we really don't want negative values or zero values
				{
					varOutput[r][chromosomeMutation][geneMutation]=varOutput[r][chromosomeMutation][geneMutation]+distribution(generator);
				}
				mutate_flag[r]=true;
			}
		}
	}
}


void tournament(vector<vector<vector<float>>> &varInput, vector<vector<vector<float>>> &varOutput, vector<float> &fitness)
{
	/* Tournament selection works as follows:
		We draw a random lottery of individuals from the population and select the best.
		The non-select are then put back in the population, and we draw another random lottery.
		This goes on until we have enough parents.
		*/
	
	// Initialize arrays and integers we'll need to do this.
	int roulette_no=NPOP*(1-TOURNEY_PROPORTION);
	int tourney_no=NPOP*TOURNEY_PROPORTION;
	
	while (roulette_no+tourney_no!=NPOP)
	{
		if (roulette_no+tourney_no>NPOP && roulette_no>tourney_no)
		{
			roulette_no+=-1;
		}
		else if (roulette_no+tourney_no>NPOP && roulette_no<tourney_no)
		{
			tourney_no+=-1;
		}
		else if (roulette_no+tourney_no<NPOP && roulette_no>tourney_no)
		{
			roulette_no+=1;
		}
		else if (roulette_no+tourney_no<NPOP && roulette_no<tourney_no)
		{
			tourney_no+=1;
		}
	}
	
	vector<bool> selection_flag (NPOP, false);
	vector<vector<int>> selection_matrix (tourney_no,vector<int>(PARENT_NO,0));
	
	// Incidentally we'll need mean and std dvn for the mutations later anyway, so:
	vector<vector<float>> meanTensor (NSECTIONS,vector<float>(NVARS,0));
	vector<vector<float>> dvnTensor (NSECTIONS,vector<float>(NVARS,0));
	
	cout << "Tournament selection initialized." << endl;
	
	for (int i=0;i<NVARS;i++)
	{
		for (int j=0;j<NSECTIONS;j++)
		{
			float totalSum=0.0f;
			for (int k=0;k<NPOP;k++)
			{
				totalSum+=varInput[k][j][i];
			}
			float mean = totalSum / NPOP;
			meanTensor[j][i]=mean;
		}
	}
	
	for (int i=0;i<NVARS;i++)
	{
		for (int j=0;j<NSECTIONS;j++)
		{
			float dvnSum=0.0f;
			for (int k=0;k<NPOP;k++)
			{
				dvnSum+=pow((varInput[k][j][i]-meanTensor[j][i]),2);
			}
			float dvn = pow((dvnSum / (NPOP - 1)),1/2);
			dvnTensor[j][i]=dvn;
		}
	}
	
	cout << "Statistics initialized." << endl;
	
	// Let's start selecting parents:
	
	vector<int> contenders (TOURNEY_LOTTERY_SIZE,0); // Array to store who we select
	vector<bool> contender_flag (NPOP, false);

	
	for(int i=0; i<tourney_no; i++)
	{
		for(int j=0; j<PARENT_NO;j++)
		{		
			int s = rand()%NPOP;
			contender_flag[s] = true;
			contenders[0] = s;
			for(int k=1; k<TOURNEY_LOTTERY_SIZE; k++)
			{
				s = rand()%NPOP;
				while(contender_flag[s] == true || selection_flag[s] == true)
					s = rand()%NPOP;
				contender_flag[s] = true;
				contenders[j] = s;
			}
			cout << "Individual " << i << " Parent " << j << " Pool selected" << endl;
			
			// Ok now we have a bunch of potential parents in the population. Now we sort through them and figure out who is the best.

			int most_fit = 0;
			for(int k=0; k<TOURNEY_LOTTERY_SIZE; k++)
			{
				if(fitness[contenders[i]]>fitness[most_fit])
					most_fit=i;
			}
			selection_matrix[i][j]=most_fit;
			selection_flag[most_fit] = true;
			
			cout << "Individual " << i << " Parent " << j << " selected." << endl;
			
			// We reset the flags for the entire parent generation EXCEPT the parents who have been selected to be parents
			
			for(int k=0; k<TOURNEY_LOTTERY_SIZE; k++)
			{
				contenders[k] = 0;
				contender_flag[k] = false;
				cout << "Contender flags reset." << endl;
			}
		}
		
		// Now we reset all flags for the entire parent generation since we're moving onto the next child
		
		for(int j=0; j<NPOP; j++)
		{
			selection_flag[j] = false;
		}
		cout << "Flags reset." << endl;
	}
	
	cout << "Tournament parents selected." << endl;
	
	// Now that we have all the parents we need to generate our child generation, we'll go ahead and generate them here:
	
	for(int i=0; i<tourney_no; i++) // this loop generates for each child
	{
		for(int j=0; j<NSECTIONS; j++) // this loop changes which chromosome we're building
		{
			for(int k=0; k<NVARS; k++) // This loop changes which gene we're slotting in
			{
				int pick1=rand()%PARENT_NO; // generates a random integer number between 0 and number of parents
				varOutput[i+roulette_no][j][k]=varInput[selection_matrix[i][pick1]][j][k]; // This looks hella messy, but really all it's saying is go to randomly chosen parent and give the offspring the allele there.
			}
		}
	}
	
	// Finally, we will introduce mutations to the offspring at random. 
	
	vector<bool> mutate_flag (tourney_no,false); // Stores if a kid has already been exposed to mutagens. No need to mutate them further
	
	// Calculate how many offspring to mutate
	
	int tour_mut = tourney_no * MUTABILITY;
	
	for(int i=0;i<tour_mut;i++)
	{ 
		int r = rand()%tourney_no; // Let's pick a random kid to expose to mutagens
		
		while(mutate_flag[r] == true) // If we already mutated that kid, pick again
			r = rand()%tourney_no;
		for(int j=0;j<NSECTIONS;j++)
		{
			int numberOfMutations = rand()%NVARS; // We apply anywhere from 1 - NVARS mutations)
			if (numberOfMutations == 0)
			{
				numberOfMutations++;
			}
			for(int k=0; k < numberOfMutations; k++)
			{
				/* This section determines the magnitude of the mutation we apply and which gene to mutate */
				int chromosomeMutation = rand()%NSECTIONS; // We randomly select which chromosome to mutate
				int geneMutation = rand()%NVARS; // We randomly select which gene to mutate
				std::default_random_engine generator;
				generator.seed(time(0));
				std::normal_distribution <float> distribution(meanTensor[chromosomeMutation][geneMutation],dvnTensor[chromosomeMutation][geneMutation]);
				
				/* This section determines whether or not the mutation adds or subtracts, and actually applies it */
				int coefficient=rand()%2;
				if(coefficient==0)
					varOutput[r][chromosomeMutation][geneMutation]=varOutput[r][chromosomeMutation][geneMutation]+distribution(generator)/MUT_MODULATOR;  // divides by MUT_MODULATOR to modulate the effect so we don't get big mutations.
				else if(coefficient==1)
					varOutput[r][chromosomeMutation][geneMutation]=varOutput[r][chromosomeMutation][geneMutation]-distribution(generator)/MUT_MODULATOR;  // divides by MUT_MODULATOR to modulate the effect so we don't get big mutations.
				while(varOutput[r][chromosomeMutation][geneMutation]<=0) // we really don't want negative values or zero values
					varOutput[r][chromosomeMutation][geneMutation]=varOutput[r][chromosomeMutation][geneMutation]+distribution(generator);
				mutate_flag[r]=true;
			}
		}
	}
}

