#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include "HiddenMarkovModel.hpp"
#include "Utils.hpp"

using namespace std;


HiddenMarkovModel::HiddenMarkovModel(const string& filename)
{
	ifstream file(filename);
	if (!file.is_open())
		throw runtime_error("file not found: " + string(filename));

	/* Create HMM based on input file, which is formatted like so:
	 *
	 * The first line contains integers N (number of states), M (number of observation symbols),
	 * and T (number of time steps or length of oberservation sequences). 
	 *
	 * The second contains each individual HMM state.
	 *
	 * The third line contains each possible observation symbol. */
	string line;

	getline(file, line);
	vector<int> sizes = split<int>(line);

	// initialize number of time steps
	_numOfTimeSteps = sizes[2];

	// get state names
	getline(file, line);
	vector<string> stateNames = split<string>(line);
	// initialize all state names
	_stateNames = stateNames;

	// get output names
	getline(file, line);
	vector<string> outputNames = split<string>(line);
	// initialize all output symbols
	_outputNames = outputNames;

	// consume "a:"
	file.ignore(numeric_limits<streamsize>::max(), '\n');

	// initialize state transition probability matrix
	for (auto i = stateNames.begin(); i != stateNames.end(); ++i)
	{
		getline(file, line);
		vector<double> curLine = split<double>(line);
		int col = 0;

		for (auto j = stateNames.begin(); j != stateNames.end(); ++j)
			_transitions[*i][*j] = curLine[col++];
	}

	// consume "b:"
	file.ignore(numeric_limits<streamsize>::max(), '\n');

	// initialize output emission probability matrix
	for (auto i = stateNames.begin(); i != stateNames.end(); ++i)
	{
		getline(file, line);
		vector<double> curLine = split<double>(line);
		int col = 0;

		for (auto j = outputNames.begin(); j != outputNames.end(); ++j)
			_emissions[*i][*j] = curLine[col++];
	}

	// consume "pi:"
	file.ignore(numeric_limits<streamsize>::max(), '\n');

	// set initial state probabilties
	getline(file, line);
	vector<double> tmp = split<double>(line);
	int col = 0;
	for (auto i = stateNames.begin(); i != stateNames.end(); ++i)
	{
		_initStates[*i] = tmp[col++];
	}
}


double HiddenMarkovModel::transition(const std::string& stt1, const std::string& stt2)
{
	// check if this state name exists as a key in our map
	if (_transitions.find(stt1) == _transitions.end())
		throw runtime_error("No such state: " + stt1);
	if (_transitions[stt1].find(stt2) == _transitions[stt1].end())
		throw runtime_error("No such state: " + stt2);

	return _transitions[stt1][stt2];
}


double HiddenMarkovModel::emission(const std::string& stt, const std::string& out)
{
	if (_emissions.find(stt) == _emissions.end())
		throw runtime_error("No such state: " + stt);
	if (_emissions[stt].find(out) == _emissions[stt].end())
		throw runtime_error("No such output: " + out);

	return _emissions[stt][out];
}


double HiddenMarkovModel::initState(const std::string& stt)
{
	if (_emissions.find(stt) == _emissions.end())
		throw runtime_error("No such state: " + stt);

	return _initStates[stt];
}


double HiddenMarkovModel::initEval(const string& out, const string& stt)
{
	return initState(stt) * emission(stt, out);
}


double HiddenMarkovModel::eval(const string& out, const string stts[2])
{
	return transition(stts[0], stts[1]) * emission(stts[1], out);
}


double HiddenMarkovModel::eval(const vector<string>& out, const vector<string>& stt)
{
	if (out.size() != stt.size())
		return 0;

	vector<string>::const_iterator curStt = stt.begin(), curOut = out.begin();
	double ret = initEval(*curOut++, *curStt);

	while (curOut != out.end())
	{
		string tmp[2] = {*curStt, *(++curStt)};
		ret *= eval(*curOut++, tmp);
	}

	return ret;
}


#if 0
/* Treat t as the time marker at each point in the observation sequence. */
double HiddenMarkovModel::forwardHelper(const vector<string>& obs, int t, const string& curStt)
{
	/* Base case: no previous paths, so the current state must be the initial state. */
	if (t == 0)
		return initEval(obs[t], curStt);

	double sum = 0;

	/* Sum up probabilities of all paths leading to curStt. */
	for (auto stt : _stateNames)
		sum += forwardHelper(obs, t-1, stt) * transition(stt, curStt);


	return sum * emission(curStt, obs[t]);
}

vector<double> HiddenMarkovModel::forward(const string& filename)
{
	/* Vector of observation sequences. */
	vector<vector<string> > observations = parseObsFile(filename);
	if (observations.empty())
		throw runtime_error("observation file is empty");

	vector<double> ret;

	/* Iterate through each sequence of observations. */
	for (auto obs : observations)
	{
		double sum = 0;

		for (auto stt : _stateNames)
			sum += forwardHelper(obs, obs.size()-1, stt);

		ret.push_back(sum);
	}

	return ret;
}
#endif
/* Treat t as the time marker at each point in the observation sequence. */
double HiddenMarkovModel::forwardHelper(const vector<string>& obs, int t, const string& curStt)
{
	/* Base case: no previous paths, so the current state must be the initial state. */
	if (t == 0)
		return initEval(obs[t], curStt);

	double sum = 0;

	/* Sum up probabilities of all paths leading to curStt. */
	for (auto stt : _stateNames)
		sum += forwardHelper(obs, t-1, stt) * transition(stt, curStt);

	return emission(curStt, obs[t]) * sum;
}

vector<double> HiddenMarkovModel::forward(const string& filename)
{
	/* Vector of observation sequences. */
	vector<vector<string> > observations = parseObsFile(filename);
	if (observations.empty())
		throw runtime_error("observation file is empty");

	vector<double> ret;

	/* Iterate through each sequence of observations. */
	for (auto obs : observations)
	{
		double sum = 0;

		for (auto stt : _stateNames)
			sum += forwardHelper(obs, obs.size()-1, stt);

		ret.push_back(sum);
	}

	return ret;
}


double HiddenMarkovModel::backwardHelper(const vector<string>& obs, int t, const string& curStt)
{
	/* Base case: no next paths, so the current state must be the final state. */
	if (t == static_cast<int>(obs.size()-1))
		return 1;

	double sum = 0;

	/* Sum up probabilities of all paths out from curStt. */
	for (auto stt : _stateNames)
		sum += transition(curStt, stt) * emission(stt, obs[t+1]) * backwardHelper(obs, t+1, stt);

	return sum;
}

vector<double> HiddenMarkovModel::backward(const string& filename)
{
	/* Vector of observation sequences. */
	vector<vector<string> > observations = parseObsFile(filename);
	if (observations.empty())
		throw runtime_error("observation file is empty");

	vector<double> ret;

	/* Iterate through each sequence of observations. */
	for (auto obs : observations)
	{
		double sum = 0;

		for (auto stt : _stateNames)
			sum += initState(stt) * emission(stt, obs[0]) * backwardHelper(obs, 0, stt);

		ret.push_back(sum);
	}

	return ret;
}


/* I sincerely hope to God no one ever reads this function definition... Here is an example of the
 * C++ code you can except when translating from Python...
 * Code taken from: https://en.wikipedia.org/wiki/Viterbi_algorithm */
pair<double, vector<string> > HiddenMarkovModel::viterbiHelper(const vector<string>& obs)
{
	map<int, map<string, double> > V;
	map<string, vector<string> > path;
	size_t t = 0;

	/* Initialize base cases (t == 0) */
	for (auto stt : _stateNames)
	{
		V[0][stt] = initState(stt) * emission(stt, obs[0]);

		vector<string> tmp = {stt};
		path[stt] = tmp;
	}

	/* Run Viterbi for t > 0. */
	++t;
	double curMaxProb = 0;
	string curMaxStt;

	while (t != obs.size())
	{
		map<string, vector<string> > newPath;

		for (auto stt_i : _stateNames)
		{
			curMaxProb = 0;

			for (auto stt_j : _stateNames)
			{
				double curr = V[t-1][stt_j] * transition(stt_j, stt_i) * emission(stt_i, obs[t]);

				if (curr > curMaxProb)
				{
					curMaxProb = curr;
					curMaxStt = stt_j;
				}
			}
			V[t][stt_i] = curMaxProb;

			vector<string> tmp1 = path[curMaxStt];
			vector<string> tmp2 = {stt_i};
			tmp1.insert(tmp1.end(), tmp2.begin(), tmp2.end());
			newPath[stt_i] = tmp1;
		}
		path = newPath; // don't need to remember the old paths

		++t;
	}

	curMaxProb = 0; // if only one element is observed, max is sought in the init values

	int n = 0;
	if (obs.size() != 1)
		n = obs.size()-1;

	for (auto stt : _stateNames)
	{
		double curr = V[n][stt];

		if (curr > curMaxProb)
		{
			curMaxProb = curr;
			curMaxStt = stt;
		}
	}

	/* Probability is zero; no such path can be built. */
	if (curMaxProb == 0)
		path.clear();

	return make_pair(curMaxProb, path[curMaxStt]);
}

vector<pair<double, vector<string> > > HiddenMarkovModel::viterbi(const string& filename)
{
	vector<vector<string> > observations = parseObsFile(filename);
	if (observations.empty())
		throw runtime_error("observation file is empty");

	vector<pair<double, vector<string> > > ret;

	/* Iterate through each sequence of observations. */
	for (auto obs : observations)
		ret.push_back(viterbiHelper(obs));

	return ret;
}


void HiddenMarkovModel::optimized(const string& obsFilename, const string& optFilename)
{
	vector<vector<string> > observations = parseObsFile(obsFilename);
	if (observations.empty())
		throw runtime_error("observation file is empty");

	ofstream file(optFilename);
	if (!file.is_open())
		throw runtime_error("cannot create file: " + optFilename);

	int N = _stateNames.size(), M = _outputNames.size(), T = _numOfTimeSteps;
	file << N << " " << M << " " << T << endl;

	/* Set with fixed floating point notation. */
	//file.setf(ios_base::fixed, ios_base::floatfield);

	/* Write state names. */
	for (auto stt : _stateNames)
		file << stt << " ";
	file << endl;

	/* Write observation symbols. */
	for (auto out : _outputNames)
		file << out << " ";
	file << endl;

	/* Write transition matrix. */
	file << "a:" << endl;
	for (auto rowStt : _stateNames)
	{
		for (auto colStt : _stateNames)
			file << expectedTransition(observations[0], rowStt, colStt) << " ";
		file << endl;
	}

	/* Write emission matrix. */
	file << "b:" << endl;
	for (auto stt : _stateNames)
	{
		for (auto out : _outputNames)
			file << expectedEmission(observations[0], stt, out) << " ";
		file << endl;
	}

	/* Write initial state matrix. */
	file << "pi:" << endl;
	for (auto stt : _stateNames)
		file << expectedInitState(observations[0], stt) << " ";
	file << endl;

	/* Unset all floating point notation flags. */
	//file.unsetf(ios_base::floatfield);
}


double HiddenMarkovModel::xi(const vector<string>& obs, int t,
							 const string& stt_i, const string& stt_j)
{
	double sum1 = forwardHelper(obs, t, stt_i) * transition(stt_i, stt_j) *
				 backwardHelper(obs, t+1, stt_j) * emission(stt_j, obs[t+1]);

	double sum2 = 0;
	for (auto stt : _stateNames)
		sum2 += forwardHelper(obs, t, stt) * backwardHelper(obs, t, stt);
	return sum1 / sum2;
}


double HiddenMarkovModel::gamma(const vector<string>& obs, int t, const string& curStt)
{
	double sum = 0;
	for (auto stt : _stateNames)
		sum += xi(obs, t, curStt, stt);
	return sum;
}


double HiddenMarkovModel::expectedTransition(const vector<string>& obs,
											 const string& stt_i, const string& stt_j)
{
	double sum1 = 0, sum2 = 0;
	for (size_t t = 0; t < obs.size()-2; ++t)
	{
		sum1 += xi(obs, t, stt_i, stt_j);
		sum2 += gamma(obs, t, stt_i);
	}
	return (sum2 == 0.0) ? 0.0 : (sum1 / sum2);
}


double HiddenMarkovModel::expectedEmission(const vector<string>& obs, const string& curStt,
										   const string& out)
{
	double sum1 = 0, sum2 = 0;
	for (size_t t = 0; t < obs.size()-1; ++t)
	{
		if (obs[t] == out)
			sum1 += gamma(obs, t, curStt);

		sum2 += gamma(obs, t, curStt);
	}
	return (sum2 == 0.0) ? 0.0 : (sum1 / sum2);
}


double HiddenMarkovModel::expectedInitState(const vector<string>& obs, const string& curStt)
{
	return gamma(obs, 0, curStt);
}
