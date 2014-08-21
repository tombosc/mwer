/*
mwer : multi-word expressions extractor
Copyright (C) 2013  Tom Bosc

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "score_calculator.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <utility>
#include <functional>
#include <numeric>

namespace mwer{
using namespace std::placeholders;
using namespace std;

/**
* @brief Intersect 2 contexts
*
* @param C1 One of the 2 contexts
* @param C2 One of the 2 contexts
*
* @return A pair of vectors containing values of matching keys
*/
pair<vector<float>, vector<float> >
ScoreCalculator::IntersectContexts(Context &C1, Context &C2)
{
	vector<float> v1;
	vector<float> v2;

	for (pair<const string, float> &wf : C1) {
		auto it = C2.find(wf.first);

		if (it != C2.end()) {
			v1.push_back(wf.second);
			v2.push_back(it->second);
		}
	}

	return make_pair(v1, v2);
}



/**
* @brief Context cos with boolean weights
*
* Boolean weight : if an type appears n times in a context, it is reduced to 1.
*
* @param C1 One of the 2 contexts
* @param C2 One of the 2 contexts
*
* @return cos(C1, C2) = C1.C2 / (||C1||*||C2||)
*/
float ScoreCalculator::ccosBool (Context &C1, Context &C2)
{
	pair<vector<float>, vector<float> > i = IntersectContexts(C1, C2);
	float product = (float) i.first.size();
	if (product == 0){
		return 0;
	} else {
		float normx = sqrt(C1.size());
		float normy = sqrt(C2.size());
		return product / (normx * normy);
	}
};



/**
* @brief Context dice with term frequency weights
*
* Term-Frequency weights : the weights are the number of times a type appears
* in a given context.
*
* @param C1 One of the 2 contexts
* @param C2 One of the 2 contexts
*
* @return dice(C1, C2) = 2*C1.C2 / (||C1||^2 + ||C2||^2)
*/
float ScoreCalculator::cdiceTF (Context &C1, Context &C2)
{
	pair<vector<float>, vector<float> > i = IntersectContexts(C1, C2);
	float product = std::inner_product(i.first.begin(), i.first.end(),
									   i.second.begin(), 0.0);
	if (product == 0){
		return 0;
	} else {
		float sq_normx = std::inner_product(i.first.begin(), i.first.end(),
											i.first.begin(), 0.0);
		float sq_normy = std::inner_product(i.second.begin(), i.second.end(),
											i.second.begin(), 0.0);
		return 2 * product / (sq_normx + sq_normy + this->smoothingParam);
	}
};




/**
* @brief Contains the association measures
*
* Guidelines to define scores :
* - when b or c divide a number, they should be smoothed (sb, sc)
* - when using a function such as std::inner_product or std::accumulate
* where you have to input initial value, be careful to input a float value
*
* @param immediate true if immediate context will be given
* @param broad true if broad context will be given
* @param toCompute vector of scores to compute
* @param smoothingParam value of the smoothing parameter
*/
ScoreCalculator::ScoreCalculator(bool immediate, bool broad,
								 vector<int> toCompute, float smoothingParam) :
	hasImmediateContext(immediate),
	hasBroadContext(broad),
	scoresToCompute(toCompute),
	smoothingParam(smoothingParam)
{
	auto min = [] (float a, float b) {
		return (a < b) ? a : b;
	};
	auto max = [] (float a, float b) {
		return (a > b) ? a : b;
	};
	scores[1] = [&] () { // joint probability
		return p_xy;
	};
	scores[2] = [&] () {
		return p_xy / p_x_star;
	};
	scores[3] = [&] () {
		return p_xy / p_star_y;
	};
	scores[4] = [&] () { // pointwise mutual information
		return log2(p_xy / p_x_star * p_star_y);
	};
	scores[5] = [&] () { // mutual dependency (MD)
		return log2(pow(p_xy, 2) / p_x_star * p_star_y);
	};
	scores[6] = [&] () { // log frequency biased MD
		return log2(pow(p_xy, 2) / (p_x_star * p_star_y)) +
			   log2(p_xy);
	};
	scores[7] = [&] () { // normalized expectation
		return 2 * a / (sb + sc);
	};
	scores[8] = [&] () { // mutual expectation
		return 2 * sa * p_xy / (sb + sc);
	};
	scores[9] = [&] () { // salience
		return log2(pow(p_xy, 2) / p_x_star * p_star_y) * log2(a);
	};
	scores[17] = [&] () { // Russel-Rao
		return a / (a + b + c + d);
	};
	scores[18] = [&] () { // Sokal-Michiner
		return (a + d) / (a + b + c + d);
	};
	scores[19] = [&] () { // Rogers-Tanimoto
		return (a + d) / (a + 2 * b + 2 * c + d);
	};
	scores[20] = [&] () { // Hamann
		return (a + d) - (b + c) / (a + b + c + d);
	};
	scores[21] = [&] () { // Third Sokal-Sneath
		return (b + c) / (a + d);
	};
	scores[22] = [&] () { // Jaccard
		return (a) / (a + b + c);
	};
	scores[23] = [&] () {
		return sa / (sb + sc); // First Kulczynsky
	};
	scores[24] = [&] () { // Second Sokal-Sneath
		return a / (a + 2 * (b + c));
	};
	scores[25] = [&] () { // Second Kulczynski
		return 0.5 * (a / (a + b) + a / (a + c));
	};
	scores[26] = [&] () { // Fourth Sokal-Sneath
		return 0.25 * (a / (a + b) + a / (a + c) + d / (d + b) + d / (d + c));
	};
	scores[27] = [&] () { // Odds ratio
		return sa * sd / (sb * sc);
	};
	scores[28] = [&] () { // Yulle's omega
		return (sqrt(sa * sd) - sqrt(sb * sc)) / (sqrt(sa * sd) + sqrt(sb * sc));
	};
	scores[29] = [&] () { // Yulle's Q
		return (a * d - b * c) / (a * d + b * c);
	};
	scores[30] = [&] () { // Driver-Kroeber
		return a / (sqrt((a + b) * (a + c)));
	};
	scores[31] = [&] () { // Fifth Sokal-Sneath
		return a * d / sqrt((a + b) * (a + c) * (d + b) * (d + c));
	};
	scores[32] = [&] () { // Pearson
		return (a * d - b * c) / sqrt((a + b) * (a + c) * (d + b) * (d + c));
	};
	scores[33] = [&] () { // Baroni-Urbani
		float s = sqrt(a * d);
		return (a + s) / (a + b + c + s);
	};
	scores[34] = [&] () { // Braun-Blanquet
		return a / max(a + b, a + c);
	};
	scores[35] = [&] () { // Simpson
		return a / min(a + b, a + c);
	};
	scores[36] = [&] () { // Michael
		return 4 * (a * d - b * c) / (pow(a + d, 2) + pow(b + c, 2));
	};
	scores[37] = [&] () { // Mountford
		return 2 * a / (2 * b * c + a * b + a * c);
	};
	scores[38] = [&] () { // Fager
		return (a / (sqrt((a + b) * (a + c)))) - 0.5 * max(b, c);
	};
	scores[39] = [&] () { // Unigram subtules
		return log2((sa * sd) / (sb * sc)) -
			   3.29 * sqrt(1 / sa + 1 / sb + 1 / sc + 1 / sd);
	};
	scores[40] = [&] () { // U cost
		return log2(1 + (min(b, c) + a) / (max(b, c) + a));
	};
	scores[41] = [&] () { // S cost
		return pow(log2(1 + (min(sb, sc) / (sa + 1))), -0.5);
	};
	scores[42] = [&] () { // R cost
		return log2(1 + a / (a + b)) * log2(1 + a / (a + c));
	};
	scores[43] = [&] () { // T combined cost
		return sqrt(scores[40]() * scores[41]() * scores[42]());
	};
	scores[44] = [&] () { // Phi
		return (p_xy - p_x_star * p_star_y) /
			   sqrt(p_x_star * p_star_y * (1 - p_x_star) * (1 - p_star_y));
	};

	if (hasImmediateContext) {
		immediateContexts.resize(2);
		const Context &cl = immediateContexts[LEFT];
		const Context &cr = immediateContexts[RIGHT];
		auto diff_product_log = [] (float x, pair<string, float> y, float contextSize) {
			float p = y.second / contextSize;
			return x - p * log2(p);
		};
		scores[57] = [&] () { // left context entropy
			return accumulate(cl.begin(), cl.end(), 0.0,
							  bind(diff_product_log, _1, _2, cl.size()));
		};
		scores[58] = [&] () { // right context entropy
			return accumulate(cr.begin(), cr.end(), 0.0,
							  bind(diff_product_log, _1, _2, cr.size()));
		};
		scores[59] = [&] () { // left context divergence
			return accumulate(cl.begin(), cl.end(), p_x_star * log2(p_x_star),
							  bind(diff_product_log, _1, _2, cl.size()));
		};
		scores[60] = [&] () { // right context divergence
			return accumulate(cr.begin(), cr.end(), p_star_y * log2(p_star_y),
							  bind(diff_product_log, _1, _2, cr.size()));
		};
	}

	if (hasBroadContext) {
		scores[62] = [&] () { // Reverse cross entropy
			auto l = [&] (float sum, pair<string, float> wf) {
				auto it = Cx->find(wf.first);
				float p = 0.0;

				if (it != Cx->end()) {
					p = it->second;
				}

				return sum + (wf.second / Cy->size()) *
					   log2((p + this->smoothingParam) / Cx->size());
			};
			return - accumulate(Cy->begin(), Cy->end(), 0.0, l);
		};
		scores[68] = [&] () { // Reverse confusion probability
			const string &t1 = typesCandidates[0].first;
			const string &t2 = typesCandidates[1].first;
			// p(x|Cz)*p(y|Cz)*p(z)
			// The broad context contains all the types which have contexts that
			// contains the types of the candidates...
			auto context_product = [&] (float sum, pair<string, float> wf) {
				Context &c = types[wf.first]->context;
				auto it1 = c.find(t1);
				auto it2 = c.find(t2);

				if (it1 == c.end() || it2 == c.end()) {
					// here, we didn't find a type in its own context...
					return sum;
				}

				// c.size() not null because we found t1 & t2
				return sum + (it1->second / c.size()) * (it2->second / c.size()) * wf.second;
			};
			return accumulate(broadContext.begin(), broadContext.end(),
							  0.0, context_product) / p_star_y;
		};
		scores[75] = [&] () { // Phrase word coocurrence
			auto occur0 = broadContext.find(typesCandidates[0].first);
			auto occur1 = broadContext.find(typesCandidates[1].first);
			float f_x_cxy = (occur0 != broadContext.end()) ? occur0->second : 0.0;
			float f_y_cxy = (occur1 != broadContext.end()) ? occur1->second : 0.0;
			return 0.5 * ((f_x_cxy / a) +  (f_y_cxy / a));
		};
		scores[77] = [&] () {
			Context &Cx = typesCandidates[0].second->context;
			Context &Cy = typesCandidates[1].second->context;
			Context &Cxy = broadContext;
			return 0.5 * (ccosBool(Cx, Cxy) + ccosBool(Cy, Cxy));
		};
		scores[81] = [&] () {
			Context &Cx = typesCandidates[0].second->context;
			Context &Cy = typesCandidates[1].second->context;
			Context &Cxy = broadContext;
			return 0.5 * (cdiceTF(Cx, Cxy) + cdiceTF(Cy, Cxy));
		};
	}

	for (auto & i : scoresToCompute) {
		if (scores.find(i) == scores.end()) {
			cerr << "Error: Function " << i << " not defined. ";
			cerr << "Replaced by null function." << endl;
			scores[i] = [] () {
				return 0.0;
			};
		}
	}
}



ScoreCalculator::~ScoreCalculator()
{
	for (auto & t : types) {
		delete t.second;
	}
}



/**
* @brief Begin entering a new candidate's infos (without contexts)
*/
void ScoreCalculator::newCandidate()
{
	broadContext.clear();
}



/**
* @brief Begin entering a new candidate's infos (with contexts)
*
* @param wordTypes types in string format of the candidates, as previously
* entered with addType()
*/
void ScoreCalculator::newCandidate(vector<string> wordTypes)
{
	newCandidate();
	typesCandidates.clear();
	typesCandidates.resize(wordTypes.size());

	for (int i = 0; i < (int) wordTypes.size(); ++i) {
		// if the file is well formed and contained all word types first
		// the types contained in candidates will for sure be found
		typesCandidates[i] = pair<string, WordTypeSimplified *>(
								 wordTypes[i], types[wordTypes[i]]);

		if (types[wordTypes[i]] == 0) {
			cout << "Error: Wordtype " << wordTypes[i] << " not found" << endl;
		}
	}

	Cx = &(typesCandidates[0].second->context);
	Cy = &(typesCandidates[1].second->context);
}



/**
* @brief Set the contingency table for current candidate
*
* @param contingencyTable
*/
void ScoreCalculator::addContingencyTable(vector<int> contingencyTable)
{
	table.assign(contingencyTable.begin(), contingencyTable.end());
	a = table[0];
	b = table[1]; // n = 2
	c = table[2]; // n = 2
	d = table[3]; // n = 2
	N = a + b + c + d; // should always keep the same value
	auto smoothContingency = [&] (float v){
		return (v + smoothingParam);
	};
	sa = smoothContingency(a);
	sb = smoothContingency(b);
	sc = smoothContingency(c);
	sd = smoothContingency(d);

	if (sb == b || sc == c) {
		cout << "Error: smoothing parameter too low : "
			 << "smoothed value = not smoothed value" << endl;
	}

	p_xy = a / N;
	p_x_star = (a + b) / N;
	p_star_y = (a + c) / N;
}




/**
* @brief Add types and frequency to immediate context
*
* @param context LEFT or RIGHT
* @param types
* @param freqs
*/
void ScoreCalculator::addToImmediateContext(ContextSide context,
											vector<string> types, vector<int> freqs)
{
	for (int i = 0; i < (int) types.size(); ++i) {
		(immediateContexts[context])[types[i]] = freqs[i];
	}
}



/**
* @brief Add types and frequency to broad context
*
* @param types
* @param freqs
*/
void ScoreCalculator::addToBroadContext(vector<string> types,
										vector<int> freqs)
{
	for (int i = 0; i < (int) types.size(); ++i) {
		broadContext[types[i]] = freqs[i];
	}
}



/**
* @brief Compute all scores that were passed to the constructor
*
* @return computed scores
*/
vector<float> ScoreCalculator::compute()
{
	vector<float> res(scoresToCompute.size());
	int newIndex = 0;

	for (int i : scoresToCompute) {
		res[newIndex] = scores[i]();
		++newIndex;
	}

	return res;
}



/**
* @brief Add a new type
*
* Name and freq have matching indexes, as well as contextNames and contextFreqs.
*
* @param name string format of the type
* @param freq global number of occurence of the type in the corpus
* @param contextNames string format of a type appearing in the context of name
* @param contextFreqs global number of occurences in the context
*/
void ScoreCalculator::addType(string name, int freq,
							  vector<string> contextNames, vector<int> contextFreqs)
{
	WordTypeSimplified *wts = new WordTypeSimplified();
	wts->freq = (float) freq;

	for (int i = 0; i < (int) contextNames.size(); ++i) {
		wts->context[contextNames[i]] = (float) contextFreqs[i];
	}

	types[name] = wts;
}
}
