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

#include <iostream>
#include <sstream>
#include <stdexcept>


namespace mwer{
using namespace std;

/**
* @brief Add a candidate
*
* @param types vector of word types
* @param parentIds parent's IDs of types, in the same order
* @param frequency number of occurences
*
* @return a pointer to the newly created candidate
*/
template<class T>
T* CandidateFilter<T>::addCandidate(vector<WordType *> types,
									  vector<int> parentIds, int frequency)
{
	T *c = new T(types, parentIds, frequency);
	auto res = candidates.insert(c);

	if (!res.second) { // the candidate already exists
		(*res.first)->updateStatistics();
		delete c;
	}
	return *res.first;
}



/**
* @brief
*
* @param n Number of word types per MWE candidates : 2 for bigrams, 3 for
* trigrams, etc...
*/
template<class T>
CandidateFilter<T>::CandidateFilter(int n) :
	n(n)
{
	if (n < 2 || n > 4) {
		throw invalid_argument("Error: n must be between 2 and 4");
	}
}



/**
* @brief Add a word type
*
* @param formOrLemma Form factor or lemma factor if available
* @param tag tag factor if available
*
* @return a pointer to the corresponding word type
*/
template<class T>
WordType *CandidateFilter<T>::addWordType(string formOrLemma,
										  string tag)
{
	WordType *type;
	type = new WordType(formOrLemma, tag);
	// Which is added to the set if it doesn't exists already
	auto res = wordTypes.insert(type);

	if (!res.second) { // WordType already exists
		delete type;
	}

	return *res.first;
}


/**
* @brief Filter all the candidates for which the factor match the regexp
*
* @param factor number of the factor to match
* @param regexp n regular expression to match, formated like this :
* regexp1:...:regexpn with n the number of word types per MWE candidates,
* and : the separator @ref SEP_REGEXPS
* @param out if true, rejects matching candidates. If false, accepts.

*/
template<class T>
void CandidateFilter<T>::regexpFilter(int factor, string regexp, bool out)
{
	vector<string> splitRegex = split(regexp, SEP_REGEXPS);

	// We check that the regex is well formated
	if ((int) splitRegex.size() != n) {
		string errMessage = "Error: Invalid regex filter form : ";
		errMessage.append(regexp);
		throw invalid_argument(errMessage);
	}

	for (auto candidate = candidates.begin(); candidate != candidates.end();) {
		bool match = (*candidate)->regexpFilter(factor, regexp);

		if ((!match && !out) || (match && out)) {
			delete *candidate;
			candidate = candidates.erase(candidate);
		} else {
			++candidate;
		}
	}
}



/**
* @brief Filter in all the candidates according to their frequencies
*
* That is, candidate is kept if c.counter belongs to {min,..,max}
*
* @param min minimal frequency for a candidate to be kept
* @param max maximal frequency for a candidate to be kept
* @param out if true, rejects matching candidates. If false, accepts.
*/
template<class T>
void CandidateFilter<T>::frequencyFilter(int min, int max, bool out)
{
	for (auto candidate = candidates.begin(); candidate != candidates.end();) {
		bool match = (*candidate)->frequencyWithinRange(min, max);

		if ((!match && !out) || (match && out)) {
			delete *candidate;
			candidate = candidates.erase(candidate);
		} else {
			++candidate;
		}
	}
}



/**
* @brief Print a list of candidate in the standard output
*/
template<class T>
void CandidateFilter<T>::printCandidates()
{
	orderedSet ordered = orderedSet(candidates.begin(), candidates.end());

	for (auto c = ordered.begin(); c != ordered.end(); ++c) {
		cout << **c << "\t" << (*c)->getFrequency() << endl;
	}
}



/**
* @brief Print a list of candidate in a file
*
* If file extension is .gz, the file will be compressed.
*
* @param filename
*/
template<class T>
void CandidateFilter<T>::writeToFile(string &filename)
{
	unique_ptr<ofstream> file(new ofstream(filename));

	bool compressed = (getExtension(filename) == ".gz");
	if (compressed) {
		unique_ptr<ostream> stream = getCompressedStream(*file);
		outputData(std::move(stream));
	} else {
		outputData(std::move(file));
	}
}




/**
 * @brief Output candidates in the stream
 *
 * @param stream
 */
template<class T>
void CandidateFilter<T>::outputData(unique_ptr<ostream> stream)
{
	orderedSet ordered = orderedSet(candidates.begin(), candidates.end());
	string sep(1, SEP_SECTIONS);

	for (auto c = ordered.begin(); c != ordered.end(); ++c) {
		*stream << **c << sep << (*c)->getFrequency() << endl;
	}

}



template<class T>
CandidateFilter<T>::~CandidateFilter()
{
	for (auto it = candidates.begin(); it != candidates.end(); ++it) {
		delete *it;
	}

	for (auto it = wordTypes.begin(); it != wordTypes.end(); ++it) {
		delete *it;
	}
}
}
