#include <fstream>
#include <limits>
#include <stdexcept>
#include "Utils.hpp"

using namespace std;


template <typename T>
vector<T> split(const string& line)
{
	vector<T> ret;
	string::const_iterator i = line.begin(), j, end = line.end();

	while (i != end && j != end)
	{
		while (isspace(*i) && i != end) ++i;
		j = i;
		while (!isspace(*j) && j != end) ++j;

		if (i != j)
			ret.push_back(string(i, j));
		i = j;
	}
	return ret;
}

/* Template specializations must be defined before the first use of that specialization.
 * C++ templates. Gah. */
template <>
vector<int> split(const string& line)
{
	vector<int> ret;
	string::const_iterator i = line.begin(), j, end = line.end();

	while (i != end && j != end)
	{
		while (isspace(*i) && i != end) ++i;
		j = i;
		while (!isspace(*j) && j != end) ++j;

		if (i != j)
		{
			string str(i, j);
			ret.push_back(strtol(str.c_str(), NULL, 10));
		}
		i = j;
	}
	return ret;
}

template <>
vector<double> split(const string& line)
{
	vector<double> ret;
	string::const_iterator i = line.begin(), j, end = line.end();

	while (i != end && j != end)
	{
		while (isspace(*i) && i != end) ++i;
		j = i;
		while (!isspace(*j) && j != end) ++j;

		if (i != j)
		{
			string str(i, j);
			ret.push_back(strtod(str.c_str(), NULL));
		}
		i = j;
	}
	return ret;
}


/* Return a vector of observation sequences from a .obs file. */
vector<vector<string> > parseObsFile(const string& filename)
{
	ifstream file(filename);
	if (!file.is_open())
		throw runtime_error("file not found: " + string(filename));

	int count;
	file >> count;
	file.ignore(numeric_limits<streamsize>::max(), '\n');

	/* Vector of observation sequences. */
	vector<vector<string> > observations(count);

	for (int i = 0; i < count; ++i)
	{
		file.ignore(numeric_limits<streamsize>::max(), '\n');

		string line;
		getline(file, line);

		observations[i] = split<string>(line);
	}
	return observations;
}
