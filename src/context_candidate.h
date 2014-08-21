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

#ifndef CONTEXT_CANDIDATE_H_
#define CONTEXT_CANDIDATE_H_

#include <set>
#include <unordered_map>
#include <vector>
#include <string>

#include "candidate.h"
#include "word_type.h"


namespace mwer{
/**
* @brief A specialization of @ref Candidate embedding contextual informations
*
* We use the term subcandidate to call candidates containing undefined types.
* They are useful to count marginal laws of candidates counts.
*/
class ContextCandidate : public Candidate {
	public:
		typedef std::unordered_map<WordType *, int> Context;

		enum ContextType {BROAD, LEFT, RIGHT};

	protected:
		std::vector<Context> contexts;

		std::set<AbstractCandidate *, CandidateLexCompare> subcandidates;

	public:
		ContextCandidate(std::vector<WordType *> v, std::vector<int> pids,
						 int f, int order = 0);

		std::ostream &output(std::ostream &);
		size_t hash() const;

		void addSubcandidate(ContextCandidate *);
		void addToContext(ContextType, WordType *);
		void updateStatistics();
		int getSize() const;
		std::string outputContingency(int N);
		std::string printContext(ContextType c);
		void substractTypesInContext();
};
}


#endif
