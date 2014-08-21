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

#include "candidate.h"
#include "shared.h"

#include <iostream>
#include <tuple>


namespace mwer{
Candidate::Candidate(std::vector<WordType *> v, std::vector<int> pid,
					 int f, int order) :
	AbstractCandidate(f),
	nW(v),
	parentIds(pid),
	order(order)
{
}



Candidate::~Candidate()
{
}



std::ostream &Candidate::output(std::ostream &os)
{
	if (parentIds.empty()) {
		for (unsigned int i = 0; i < nW.size() - 1; ++i) {
			os << *nW[i] << " ";
		}

		os << *nW[nW.size() - 1];
	} else {
		for (unsigned int i = 0; i < nW.size() - 1; ++i) {
			os << *nW[i] << "|" << i + 1 << "|" << parentIds[i] << " ";
		}

		os << *nW[nW.size() - 1] << "|" << nW.size() << "|" << parentIds[nW.size() - 1];
	}

	return os;
}



size_t Candidate::hash() const
{
	WordTypeHash h;
	size_t sum = 0;

	for (int i = 0; i < (int) nW.size(); ++i) {
		sum ^= (h(nW[i]) << i);
	}

	return sum;
}



bool Candidate::compare(const AbstractCandidate &ac) const
{
	const Candidate &c = static_cast<const Candidate &>(ac);

	if (order != c.order) {
		return false;
	}

	return nW == c.nW && parentIds == c.parentIds;
}



/**
 * This implementation add the notion of order of a candidate. The order
 * is the first discriminant parameter to order.
 */
bool Candidate::operator< (const AbstractCandidate &ac) const
{
	const Candidate &c = static_cast<const Candidate &> (ac);

	if (order < c.order) {
		return true;
	} else if (order > c.order) {
		return false;
	}

	for (unsigned int i = 0; i < nW.size(); ++i) {
		if (nW[i] == 0 && c.nW[i] != 0) {
			return false;
		} else if (nW[i] != 0 && c.nW[i] == 0) {
			return true;
		} else if (nW[i] != 0 && c.nW[i] != 0) {
			if (*nW[i] < *c.nW[i]) {
				return true;
			} else if (!(*nW[i] == *c.nW[i])) {
				return false;
			}
		}
	}

	// if we are there, it means the types are the same
	// so there must be a tree structure to discriminate on
	return parentIds < c.parentIds;
}



bool Candidate::regexpFilter(int factor, std::string regexp)
{
	std::vector<std::string> regexps = split(regexp, SEP_REGEXPS);

	for (unsigned int i = 0; i < nW.size(); i++) {
		if (factor == FORM || factor == LEMMA) {
			std::string &s = nW[i]->getFormOrLemma();

			if (!contains(s, regexps[i])) {
				return false;
			}
		} else if (factor == TAG) {
			std::string &s = nW[i]->getTag();

			if (!contains(s, regexps[i])) {
				return false;
			}
		}
	}

	return true;
}
}


