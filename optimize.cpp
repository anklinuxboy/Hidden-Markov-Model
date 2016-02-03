#include <algorithm>
#include <fstream>
#include <iostream>
#include "HiddenMarkovModel.hpp"

using namespace std;


void help(char*);


int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		help(argv[0]);
		return 1;
	}

	/* Parse arguments. We accept only one .hmm file and one .obs file. */
	string hmmFilename, obsFilename, optHmmFilename;

	for (int i = 1; i < argc; ++i)
	{
		string arg(argv[i]);

		if (arg.find(".hmm") != string::npos)
		{
			if (hmmFilename.empty())
				hmmFilename = arg;
			else
				optHmmFilename = arg;
		}
		else if (arg.find(".obs") != string::npos)
			obsFilename = arg;
	}

	if (hmmFilename.empty())
	{
		cerr << "no .hmm file found" << endl;
		return 1;
	}
	if (obsFilename.empty())
	{
		cerr << "no input .obs file found" << endl;
		return 1;
	}
	if (optHmmFilename.empty())
	{
		cerr << "no output .obs file found" << endl;
		return 1;
	}


	HiddenMarkovModel hmm(hmmFilename);
	cout << hmm.forward(obsFilename)[0];

	hmm.optimized(obsFilename, optHmmFilename);

	HiddenMarkovModel optimized(optHmmFilename);
	cout << " " << optimized.forward(obsFilename)[0] << endl;

	return 0;
}


void help(char* program)
{
	cout << program << ": [model.hmm] [observation.obs] [optimized_model.hmm]" << endl;
}
