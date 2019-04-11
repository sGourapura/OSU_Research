
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
/* Here we declare our global variables. */
int NPOP; /* This global constant represents how many individuals there are per generation. It's value is determined by the user, in argv[1]. It cannot be cast as a constant, because the user's value of NPOP has to be defined inside int main. BE VERY CAREFUL NOT TO REDEFINE NPOP SOMEWHERE IN THE CODE! */
const int HEADER = 729; // Which line is Veff(ice) on from the AraSim output files?
const int NLINES = 737; // How many lines are there in the AraSim output files?
const double EPSILON = 0.000001; // Catches errors. If the first individual's fitness score is less than this value, we rerun the program.
/* Here we declare our function headers. */
void Read(char* filename, ifstream& inputFile, string* araLineArray, vector<double> &fitnessScores, int individualCounter);
void WriteFitnessScores(vector<double> fitnessScores);
/** MAIN FUNCTION **/
int main(int argc, char** argv)
{
	string currentLine = "test Veff(ice) : 3.74466e+10 m3sr, 37.4466 km3sr";

	int commaToken=currentLine.find(",");
    int spaceToken=currentLine.find(" ",commaToken+2);
    cout << "re" << endl;
        
    string vEff = currentLine.substr(commaToken + 2, (spaceToken-commaToken-1));
    cout << vEff << endl;

return 0;
}
