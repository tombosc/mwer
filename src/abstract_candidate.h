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

#ifndef ABSTRACT_CANDIDATE_H_
#define ABSTRACT_CANDIDATE_H_

#include <iostream>
#include <string>

namespace mwer{
/**
* @brief An abstract candidate
*
* A candidate is a potential MWE expression. This class defines basic
* functions that every candidate has to implement.
*
* Each candidate also embed a counter which counts the number of
* occurence that has been met.
*
* A candidate is constituted of several factors, typically obtained after
* an annotation process : part-of-speech tagging, lemmatization, etc...
*
*/
class AbstractCandidate {
	protected:
		/**
		* @brief Counts the number of times the candidate has been met.
		*/
		int counter;
	public:
		AbstractCandidate(int counter = 1);
		virtual ~AbstractCandidate() = 0;
		/**
		 * @brief Output the candidate in an output stream
		 *
		 * @param stream stream to output to
		 */
		virtual std::ostream &output(std::ostream & stream) = 0;
		/**
		* The efficiency of the whole process depends heavily on this, because it
		* impacts the storage of the candidates and the speed to access candidates.
		*
		* @return Hash
		*/
		virtual size_t hash() const = 0;
		/**
		 * @brief Apply regex on a given factor
		 *
		 * @param nFactors Number of the factor
		 * @param regexp regex to apply
		 *
		 * @return True if the candidate's factor n°nFactors matches 
		 * the regex regexp
		 */
		virtual bool regexpFilter(int nFactors, std::string regexp) = 0;
		/**
		* @param ac The candidate to whom to compare
		*
		* @return true if the ac is equivalent to the object (*this).
		*	false otherwise.
		*/
		virtual bool compare(const AbstractCandidate &ac) const = 0;

		/**
		* @brief lexicographic order
		*
		* @param ac candidate to compare to
		*
		* @return true if the object (*this) is placed before ac in lexicographic order
		*/
		virtual bool operator< (const AbstractCandidate &ac) const = 0;


		/**
		* It is called when a candidate is met (for the first time, or not).
		*/
		virtual void updateStatistics();

		/**
		* @param min
		* @param max
		*
		* @return true if the candidate has been met a number of times
		* between min and max included
		*/
		bool frequencyWithinRange(int min, int max);

		int getFrequency() const;


};

/**
* @brief A hash functor for pointers of candidates relying on @ref AbstractCandidate::hash()
*/
struct CandidateHash {
	size_t operator()(AbstractCandidate *c) const;
};



/**
* @brief An equality test functor for pointers of candidates relying on @ref AbstractCandidate::compare()
*/
struct CandidateEq {
	bool operator()(const AbstractCandidate *c1,
					const AbstractCandidate *c2) const;
};


/**
* @brief A comparator functor providing strict weak ordering
*/
struct CandidateLexCompare {
	bool operator()(const AbstractCandidate *c1,
					const AbstractCandidate *c2) const;
};

std::ostream &operator<<(std::ostream &os, AbstractCandidate &t);
}



#endif

