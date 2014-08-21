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

#include "context_candidate.h"

#include <sstream>

namespace mwer{
ContextCandidate::ContextCandidate(std::vector<WordType *> v,
								   std::vector<int> pid, int f, int order) :
	Candidate(v, pid, f, order)
{
	if (order == 0) {
		contexts.resize(3);
	} else {
		contexts.resize(1);
	}
}



std::ostream& ContextCandidate::output(std::ostream &os)
{
	if (parentIds.empty()) {
		for (unsigned int i = 0; i < nW.size(); ++i) {
			if (nW[i] != 0) { // for debugging purposes
				os << *nW[i];
			} else {
				os << "*";
			}

			if (i != nW.size() - 1) {
				os << " ";
			}
		}
	} else {
		for (unsigned int i = 0; i < nW.size(); ++i) {
			if (nW[i] != 0) { // idem
				os << *nW[i] << "|" << i + 1 << "|" << parentIds[i];
			} else {
				os << "*";
			}

			if (i != nW.size() - 1) {
				os << " ";
			}
		}
	}

	return os;
}



/**
* @brief Returns the contingency table
*
* @param N Total number of candidates observed
*
* Description of the format :
* each space separated string is an occurence count
* a, b, c, d are word types, whereas A, B, C, D are any word types
* except respectively a, b, c, d.
*
* example : aB is the number of times a has been followed by
* anything but b in a candidate

* n = 2 : ab aB Ab AB
* n = 3 : abc abC aBc Abc aBC AbC ABc ABC
* n = 4 : abcd abcD abCd aBcd Abcd abCD aBcD aBCd AbcD AbCd ABcd aBCD AbCD ABcD ABCd ABCD
*
* @return Contingency table
*/
std::string ContextCandidate::outputContingency(int N)
{
	int sum_abcd = counter;
	std::string s = std::to_string(counter);

	for (auto it = subcandidates.begin(); it != subcandidates.end(); ++it) {
		s.append(" ");
		s.append(std::to_string((*it)->getFrequency() - counter));
		sum_abcd += (*it)->getFrequency() - counter;
	}

	s.append(" ");
	s.append(std::to_string(N - sum_abcd));
	return s;
}



/**
* @brief Returns a formatted version of the context c
*
* @param c context to output
*/
std::string ContextCandidate::printContext(ContextType c)
{
	Context &context = contexts[c];
	std::stringstream ss;

	for (auto it = context.begin(); it != context.end() ; ++it) {
		ss << *it->first << ":" << it->second << " ";
	}

	std::string s = ss.str();

	if (!s.empty()) {
		// C++11 version
		//s.pop_back();
		s.resize(s.size() - 1);
	}

	return s;
}



size_t ContextCandidate::hash() const
{
	WordTypeHash h;
	size_t sum = 0;

	for (int i = 0; i < (int) nW.size(); ++i) {
		if (nW[i] != 0) {
			sum ^= (h(nW[i]) << i);
		}
	}

	return sum;
}



/**
* @brief store a subcandidate
*
* We do not check if it's already been inserted, but it is supposed to be once.
*
* @param c subcandidate
*/
void ContextCandidate::addSubcandidate(ContextCandidate *c)
{
	subcandidates.insert(c);
}



/**
* @brief Add type to the context c
*/
void ContextCandidate::addToContext(ContextType c, WordType *type)
{
	Context &context = contexts[c];
	++context[type];
}


/**
* @brief Update the statistics of the candidate as if it had been seen.
*
* Upgrade recursively every subcandidate
*
*/
void ContextCandidate::updateStatistics()
{
	++counter;

	for (auto it = subcandidates.begin(); it != subcandidates.end(); ++it) {
		(*it)->updateStatistics();
	}
}



int ContextCandidate::getSize() const
{
	return nW.size();
}



/**
* @brief Fix the insertion in broad context of its own types
*
* Substracts its types from its broad context the number of times
* the candidate has been encountered.
*
* This function is an artificial way to correct insertion of itself
* in its own broad context.
*/
void ContextCandidate::substractTypesInContext()
{
	Context &broad = contexts[BROAD];

	for (auto & t : nW) {
		auto i = broad.find(t);

		if (i != broad.end()) {
			i->second -= counter;

			if (i->second == 0) {
				broad.erase(i);
			}
		}
	}
}
}
