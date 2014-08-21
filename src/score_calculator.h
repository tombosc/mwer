#ifndef SCORE_CALCULATOR_H_
#define SCORE_CALCULATOR_H_

#include "word_type.h"

#include <vector>
#include <unordered_map>

namespace mwer{
/**
* @brief A calculator processing statistics into scores
*
* ScoreCalculator should be seen as a state machine. Before adding contingency
* table and context, you should call newCandidate().
*
*/
class ScoreCalculator {
	private:
		typedef std::unordered_map<std::string, float> Context;
		typedef std::function<float()> Score;
		struct WordTypeSimplified {
			float freq;
			Context context;
		};

		bool hasImmediateContext;
		bool hasBroadContext;
		std::vector<int> scoresToCompute;
		float smoothingParam;

		std::vector<Context> immediateContexts;
		Context broadContext;

		std::vector<float> table; // contingency table
		std::unordered_map<int, Score> scores;

		std::unordered_map<std::string, WordTypeSimplified *> types;
		std::vector < std::pair < std::string,
			WordTypeSimplified * > > typesCandidates;
		float p_xy; // p(xy);
		float p_x_star;	// p(x*)
		float p_star_y; // p(*y)
		float a, b, c, d; // contingency table values
		float sa, sb, sc, sd; // smoothed values
		float N; // total number of candidates
		Context *Cx;
		Context *Cy;

		// Context utilities
		std::pair<std::vector<float>, std::vector<float> >
		IntersectContexts(Context &, Context &);
		float ccosBool(Context &, Context &);
		float cdiceTF(Context &, Context &);

	public:
		enum ContextSide {LEFT = 0, RIGHT = 1};

		ScoreCalculator(bool immediate, bool broad,
						std::vector<int> toCompute, float smoothingParam);
		~ScoreCalculator();

		void newCandidate();
		void newCandidate(std::vector<std::string> wordTypes);
		void addContingencyTable(std::vector<int> contingencyTable);
		void addToImmediateContext(ContextSide context,
																std::vector<std::string> types, 
																std::vector<int> freqs);
		void addToBroadContext(std::vector<std::string> types,
			 										std::vector<int> freqs);
		std::vector<float> compute();
		void addType(std::string name, int freq,
			 					std::vector<std::string> contextNames,
							 	std::vector<int> contextFreqs);
};
}



#endif
