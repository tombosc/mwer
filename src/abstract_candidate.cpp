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

#include "abstract_candidate.h"


namespace mwer{
/**
* @brief
*
* @param counter Usually, when we encounter a candidate, we want to count
* one occurence. It's useful to be able to start at 0 or at another value
* if the number of occurence has already been counted.
*/
AbstractCandidate::AbstractCandidate(int counter) :
	counter(counter)
{
}



AbstractCandidate::~AbstractCandidate()
{
}



bool AbstractCandidate::frequencyWithinRange(int min, int max)
{
	return (counter >= min && counter <= max);
}



/**
 * @brief 
 *
 * @return the number of time the candidate has been met
 */
int AbstractCandidate::getFrequency() const
{
	return counter;
}



void AbstractCandidate::updateStatistics()
{
	++counter;
}



size_t CandidateHash::operator()(AbstractCandidate *c) const
{
	return c->hash();
}



bool CandidateEq::operator()(const AbstractCandidate *c1,
							 const AbstractCandidate *c2) const
{
	return c1->compare(*c2);
}



bool CandidateLexCompare::operator()(const AbstractCandidate *c1,
									 const AbstractCandidate *c2) const
{
	return *c1 < *c2;
}



std::ostream &operator<<(std::ostream &os, AbstractCandidate &t)
{
	return t.output(os);
}
}
