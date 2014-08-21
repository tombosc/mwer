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

#include "statistic_extractor.h"

#include <map>
#include <algorithm>

#include "candidate.h"

namespace mwer{
using namespace std;

long long StatisticExtractor::N = 0;

/**
* @brief Build a statistic extractor
*
* @param n Number of word types contained in a MWE candidate
* @param nFactors Number of factors per token
* @param surfMin Minimal distance of the two furthest tokens in a candidate
* @param surfMax Maximal distance of the two furthest tokens in a candidate
* @param dependency If true, the extractor will search for candidates using
* syntaxic informations (that have to be provided in this case).
* if false, the extractor will search for candidates based on the position
* of the tokens in the sentence
* @param immediate if true, immediate context (left & right types of
* the expression) will be extracted
* @param broad if true, broad context (types contained in the sentence) will 
* be extracted
* @param tagFilter Tag filter on context members (for immediate or broad set
* to true)
*/
StatisticExtractor::StatisticExtractor(int n, int nFactors, int surfMin,
									   int surfMax, bool dependency, bool immediate, bool broad,
									   std::string tagFilter) :
	CandidateExtractor<ContextCandidate>(n, nFactors, surfMin, surfMax, dependency),
	immediateContext(immediate),
	broadContext(broad),
	subcandidates(n - 1),
	filterContext(!tagFilter.empty()),
	tagFilter(tagFilter)
{
}



StatisticExtractor::~StatisticExtractor()
{
	for (auto &sc : subcandidates) {
		for (auto &c : sc) {
			delete c;
		}
	}
}



/**
* @brief Update broad context of candidate in argument.
*
* It add to the context all the types contained in the sentence, even 
* the types contained in the candidate itself. Thus, we need a function
* such as finnish()
*
* @param candidate
*/
void StatisticExtractor::updateBroadContext(ContextCandidate *candidate)
{
	for (auto t = sentence.begin() + 1; t != sentence.end(); ++t) {
		WordType *wt = (*t)->getWordType();

		if (canAddToContext(wt)) {
			candidate->addToContext(ContextCandidate::BROAD, (*t)->getWordType());
		}
	}
}



/**
* @brief update statistics of a possible candidate
*
* The candidate matching tokens will be searched. If it exists, it will
* update its statistics.
*
* If the candidate has been inserted but has not been encounter in the text
* yet, it will create its subcandidates.
*
* @param types types forming a possible candidate
*	@param pids parent's IDs
* @param unused
* @param tPrev type corresponding to the token on the left of the occurence
* of the candidate
* @param tNext type corresponding to the token on the rught of the occurence
* of the candidate
*/
void StatisticExtractor::computeStats(vector<WordType *> types,
									  vector<int> pids,
									  int unused, WordType *tPrev, WordType *tNext)
{
	// we do not add the candidate, it should already exist
	// if it doesn't exist, it means that it has been filtered out
	// so it means we don't want to consider it
	ContextCandidate *c = new ContextCandidate(types, pids, 0);
	auto res = candidates.find(c);

	if (res != candidates.end()) {
		delete c;
		c = *res;

		if (immediateContext) {
			if (tPrev != 0 && canAddToContext(tPrev)) {
				c->addToContext(ContextCandidate::LEFT, tPrev);
			}

			if (tNext != 0 && canAddToContext(tNext)) {
				c->addToContext(ContextCandidate::RIGHT, tNext);
			}
		}

		if (broadContext) {
			updateBroadContext(c);
		}

		c->updateStatistics();
		++StatisticExtractor::N;
	} else {
		delete c;
	}
}



/**
* @brief Return true if the type can be added to a context.
*
* If there are no tag filter, of if it matches the tag filter, it returns True.
*
* @param type
*
* @return 
*/
inline bool StatisticExtractor::canAddToContext(WordType *type)
{
	return !filterContext ||
		   (filterContext && contains(type->getTag(), tagFilter));
}



void StatisticExtractor::updateStatistics()
{
	cb_candidate f = bind(&StatisticExtractor::computeStats,	this,
						  _1, _2, _3, _4, _5);

	if (extractDependency) {
		computeDepCandidates(f);
	} else if ((int) this->sentence.size() > n) {
		// sentence must be long enough to process surface candidates
		computeSurfCandidates(f);
	}

	if (broadContext) {
		// Compute context of one-type subcandidates
		// This can't be done through the compute...Candidates functions because
		// the context would be added several times if a type appears in several
		// candidates
		ContextCandidate *unigram;
		WordType *type;

		for (auto tok = sentence.begin() + 1;
				tok != sentence.end(); ++tok) {
			type = (*tok)->getWordType();
			// We create a one-type subcandidate that we either add or fetch
			unigram = new ContextCandidate({type}, {}, 0);
			auto res = unigrams.insert(unigram);

			if (!res.second) {
				delete unigram;
			}

			unigram = *res.first;
			// We update its stats and add every other types to its context
			unigram->updateStatistics();

			for (auto t = sentence.begin() + 1;
					t != sentence.end(); ++t) {
				WordType *wt = (*t)->getWordType();

				if (not(*(wt) == *type) && canAddToContext(wt)) {
					unigram->addToContext(ContextCandidate::BROAD, wt);
				}
			}
		}
	}

	// clear the sentences informations
	for (auto it = this->sentence.begin() + 1;
			it != this->sentence.end(); ++it) {
		delete *it;
	}

	this->sentence.clear();
	this->sentence.push_back(&nullToken);
}



/**
* @brief Add a candidate & its subcandidate
*
* For more info, see base class' function
*
* @param types
* @param parentIds
* @param frequency
* 
* @return 
*/
ContextCandidate* StatisticExtractor::addCandidate(vector<WordType *> types,
									  vector<int> parentIds, int frequency){
		ContextCandidate* c;
		c = CandidateFilter<ContextCandidate>::addCandidate(types, parentIds, 0);
		addSubcandidates(c, types, 0);
		return c;
}



/**
* @brief add every subcandidate to the container subcandidates
*
* @param types Types composing the candidate
*
* @return every subcandidate
*/
void StatisticExtractor::addSubcandidates(ContextCandidate *candidate
										  , vector<WordType *> types, int order)
{
	// NOTE : This function is unnecessarily written recursively. To rewrite

	int n = candidate->getSize();

	for (int i = 0; i < n; ++i) {
		if (types[i] != 0) {
			vector<WordType *> t = types;
			t[i] = 0;
			ContextCandidate *c = new ContextCandidate(t, {}, 0, order);
			auto res = subcandidates[order].insert(c);

			if (!res.second) { // subcandidate already exists
				delete c;
			}

			candidate->addSubcandidate(*res.first);

			if (order < n - 2) {
				addSubcandidates(candidate, t, order + 1);
			}
		}
	}
}



/**
* @brief Output statistics in a stream
*
* @param stream
*/
void StatisticExtractor::outputData(unique_ptr<ostream> stream)
{
	string sep(1, SEP_SECTIONS);

	if (broadContext) {
		auto orderedUnigrams = orderedSet(unigrams.begin(), unigrams.end());

		for (auto u = orderedUnigrams.begin(); u != orderedUnigrams.end(); ++u) {
			*stream << **u << sep << (*u)->getFrequency();
			*stream << sep << (*u)->printContext(ContextCandidate::BROAD) << endl;
		}
	}

	auto orderedCandidates = orderedSet(candidates.begin(), candidates.end());

	for (auto c = orderedCandidates.begin(); c != orderedCandidates.end(); ++c) {
		*stream << **c << sep << (*c)->outputContingency(StatisticExtractor::N);

		if (immediateContext) {
			*stream << sep << (*c)->printContext(ContextCandidate::LEFT);
			*stream << sep << (*c)->printContext(ContextCandidate::RIGHT);
		}

		if (broadContext) {
			*stream << sep << (*c)->printContext(ContextCandidate::BROAD);
		}

		*stream << endl;
	}
}



/**
* @brief Post-process the results to terminate the processing
*
* This function must be called. If not called, the statistics are not corrects.
*
*/
void StatisticExtractor::finish()
{
	// First correction : Broad context of candidates
	for (auto & c : candidates) {
		c->substractTypesInContext();
	}
}
}
