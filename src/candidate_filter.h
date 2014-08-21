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

#ifndef CANDIDATE_FILTER_H_
#define CANDIDATE_FILTER_H_

#include <string>
#include <unordered_set>
#include <set>
#include <utility>
#include <iostream>
#include <fstream>

#include "word_type.h"
#include "candidate.h"
#include "shared.h"

namespace mwer{
/**
* @brief A frequency and regexp filter for candidates
*
* First, insert the word types that are used by candidates with
* @ref addWordType and the candidates one after the other with @ref
* addCandidate.

* Then, you'll be able to filter on a chosen factor using @ref regexpFilter,
* or filter in candidates with frequencies within a specified range
* with @ref frequencyFilter.
*
* @tparam T Object of type derived from @ref AbstractCandidate
*/
template<class T>
class CandidateFilter {
	protected:
		// number of word types per MWE candidates
		int n;

		// word types and candidates storage
		std::unordered_set<T *, CandidateHash, CandidateEq> candidates;
		std::unordered_set<WordType *, WordTypeHash, WordTypeEq> wordTypes;

		virtual void outputData(std::unique_ptr<std::ostream>);

	public:
		typedef std::set<T *, CandidateLexCompare> orderedSet;

		CandidateFilter(int n);
		virtual ~CandidateFilter();

		WordType *addWordType(std::string formOrLemma,
							  std::string tag = std::string());
		virtual T* addCandidate(std::vector<WordType *> types,
						  std::vector<int> parentIds = std::vector<int>(),
						  int frequency = 1);

		void regexpFilter(int factor, std::string regexp, bool out = false);
		void frequencyFilter(int min, int max, bool out = false);

		orderedSet orderCandidates();
		void printCandidates();
		void writeToFile(std::string &s);
};
}

#include "candidate_filter.tpp"

#endif
