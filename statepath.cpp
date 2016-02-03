#include <algorithm>
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

	/* Parse arguments. We accept only one .hmm file but allow multiple .obs files. */
	string hmmFilename;
	vector<string> obsFilenames;

	for (int i = 1; i < argc; ++i)
	{
		string arg(argv[i]);

		if (arg.find(".hmm") != string::npos)
			hmmFilename = arg;
		else if (arg.find(".obs") != string::npos)
			obsFilenames.push_back(arg);
	}

	if (hmmFilename.empty())
	{
		cerr << "no .hmm file found" << endl;
		return 1;
	}

	HiddenMarkovModel hmm(hmmFilename);

	/* Evaluate Viterbi algorithm for each .obs file. Each file may have multiple sequences. */
	for (auto i = obsFilenames.begin(); i != obsFilenames.end(); ++i)
	{
		cout << *i << ":" << endl;

		/* Print the statepath results for each observation in this file. */
		for (auto result : hmm.viterbi(*i))
		{
			cout << result.first;

			const vector<string>& path = result.second;
			for_each(path.begin(), path.end(), [](const string& s) { cout << " " << s; });

			cout << endl;
		}
	}

	return 0;
}


void help(char* program)
{
	cout << program << ": [model.hmm] [observation.obs ...]" << endl;
}
