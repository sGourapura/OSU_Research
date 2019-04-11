
#include <iostream>
#include <string>
#include <stdlib.h>
using namespace std;

int NPOP;
void testFcn();


int main(int argc, char* argv[])
{
	testFcn();
	cout << "You have entered " << argc << " arguments." << endl;
//	cout << argcb;
	for (int i = 0; i < argc; i++){
		cout << argv[i] << endl;
	}
	NPOP = atoi(argv[2]);

	if (NPOP == 10){
		cout << "true" << endl;
	}
	testFcn();
	return 0;
}

void testFcn()
{
	cout << NPOP;
}
