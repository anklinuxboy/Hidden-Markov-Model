#ifndef GUARD_HMM_HPP
#define GUARD_HMM_HPP

#include <map>
#include <string>
#include <vector>


/*
 * Good references for the underlying algorithms:
 * - L. R. Rabiner. A Tutorial on Hidden Markov Models and Selected Applications in Speech 
 * Recognition, Proceedings of the IEEE, 77(2), pp. 257-286, 1989.
 * - https://www.comp.leeds.ac.uk/roger/HiddenMarkovModels/html_dev/main.html
 * - http://www.shokhirev.com/nikolai/abc/alg/hmm/hmm.html
 * - Wikipedia
 */
class HiddenMarkovModel
{
public:
	HiddenMarkovModel(const std::string& filename);

	const std::vector<std::string>& states() const { return _stateNames; }
	const std::vector<std::string>& outputs() const { return _outputNames; }
	const int timeSteps() const { return _numOfTimeSteps; }

	/**
	 * Return state transition probability from states stt1 to stt2.
	 * @param stt1 source state
	 * @param stt2 destination state
	 */
	double transition(const std::string& stt1, const std::string& stt2);
	/**
	 * Return observation emission probability of output out in state stt.
	 * @param stt current state
	 * @param out the output symbol observed at this state
	 */
	double emission(const std::string& stt, const std::string& out);
	/**
	 * Return initial state probability of state stt.
	 * @param stt current state
	 */
	double initState(const std::string& stt);

	/**
	 * Returns initial probability of starting in a state.
	 */
	double initEval(const std::string& out, const std::string& stt);
	/**
	 * Returns probability of a single output symbol and a state transition.
	 */
	double eval(const std::string& out, const std::string stts[2]);
	/**
	 * Returns probability of an output sequence based on a given state sequence.
	 */
	double eval(const std::vector<std::string>& out, const std::vector<std::string>& stt);

	/**
	 * Returns the forward variables for each observation sequence in a given .obs file.
	 */
	std::vector<double> forward(const std::string& filename);
	/**
	 * Returns the backward variables for each observation sequence in a given .obs file.
	 */
	std::vector<double> backward(const std::string& filename);
	/**
	 * Returns the pair of the most likely state sequence probability and its actual state path
	 * for each observation sequence in a given .obs file.
	 */
	std::vector<std::pair<double, std::vector<std::string> > > viterbi(const std::string& filename);
	/**
	 * Writes an optimized HMM with respect to a given observation sequence in an .obs file.
	 */
	void optimized(const std::string& obsFilename, const std::string& optFilename);

private:
	double forwardHelper(const std::vector<std::string>&, int, const std::string&);
	double backwardHelper(const std::vector<std::string>&, int, const std::string&);
	std::pair<double, std::vector<std::string> > viterbiHelper(const std::vector<std::string>&);

	double xi(const std::vector<std::string>&, int, const std::string&, const std::string&);
	double gamma(const std::vector<std::string>&, int, const std::string&);

	double expectedTransition(const std::vector<std::string>&,
							  const std::string&, const std::string&);
	double expectedEmission(const std::vector<std::string>&,
							const std::string&, const std::string&);
	double expectedInitState(const std::vector<std::string>&, const std::string&);

private:
	size_t _numOfTimeSteps;
	std::vector<std::string> _stateNames, _outputNames;

	std::map<std::string, std::map<std::string, double> > _transitions;
	std::map<std::string, std::map<std::string, double> > _emissions;
	std::map<std::string, double> _initStates;
};


#endif
