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

#ifndef STATISTIC_EXTRACTOR_H
#define STATISTIC_EXTRACTOR_H

#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

#include "candidate_extractor.h"
#include "context_candidate.h"
#include "word_type.h"
#include "token.h"

namespace mwer{
/**
* @brief A statistic extractor
*
* To extract statistics, you first need to insert all the candidates (read from
* a list, or straight from a corpus). This is done using inherited methods.
* Then you will read the corpus in order to update statistics for every
* candidates and subcandidates. This class implements such methods.
*/
class StatisticExtractor : public CandidateExtractor<ContextCandidate> {
	private:
		typedef std::unordered_set< ContextCandidate *, CandidateHash, CandidateEq > candidate_set;
		static long long N; // Total encountered candidates
		bool immediateContext;
		bool broadContext;
		std::vector< candidate_set > subcandidates;
		bool filterContext;
		std::string tagFilter;

		candidate_set unigrams;

		void addSubcandidates(ContextCandidate *, std::vector<WordType *>,
							  int order);

		void computeStats(std::vector<WordType *> tokens, std::vector<int> pids,
						  int unused, WordType *t1, WordType *t2);

		void updateBroadContext(ContextCandidate *);
		bool canAddToContext(WordType *);
		void outputData(std::unique_ptr<std::ostream>);

	public:
		StatisticExtractor(int n, int nFactors, int surfMin, int surfMax,
						   bool dependency, bool immediate, bool broad,
						   std::string tagFilter);
		~StatisticExtractor();

		virtual ContextCandidate* addCandidate(std::vector<WordType *> t,
											  std::vector<int> pids, int f = 0);
		void updateStatistics();
		void finish();
};
}

#endif
