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

#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include <vector>
#include <string>
#include <iostream>

#include "abstract_candidate.h"
#include "word_type.h"

namespace mwer{
/**
* @brief An implementation of an @ref AbstractCandidate, with optional
* syntactic dependencies informations.
*
* This implementation encodes the id and parent id of the word types, thus
* allowing us to build a tree-like representation of candidates.
* Such a representation can be used for dependency candidates extraction.
*
*/
class Candidate : public AbstractCandidate {
	protected:
		std::vector<WordType *> nW;
		std::vector<int> parentIds;

		/**
		* @brief the candidates contains (n - order) types
		*/
		int order;
	public:
		Candidate(std::vector<WordType *> v, std::vector<int> pid, int freq = 1,
				  int order = 0);
		~Candidate();

		// inherited from AbstractCandidate
		std::ostream &output(std::ostream &);
		size_t hash() const;
		bool compare(const AbstractCandidate &) const;
		bool operator<(const AbstractCandidate &a) const;
		bool regexpFilter(int nFactors, std::string regexp);

};
}

#endif

