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

#ifndef CANDIDATE_EXTRACTOR_H_
#define CANDIDATE_EXTRACTOR_H_

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <type_traits>

#include "candidate_filter.h"
#include "context_candidate.h"
#include "word_type.h"
#include "token.h"
#include "tree.h"
#include "shared.h"

#define MAX_WORDS_PER_SENTENCE 1024

#define DEBUG_EXTR 0

#if DEBUG_EXTR
#define TRACE(args) std::cout<<args<<std::endl
#else
#define TRACE(args)
#endif

namespace mwer{
/**
* @brief A candidate extractor
*
* The distance of a candidate is defined by the maximal distance between two
* of its tokens position in a sentence. Each candidate's distance should
* belong to the range defined by {surfMin,..,surfMax}. Otherwise, the candidate
* is not stored.
*
* There are two ways of finding candidates : with or without syntactic
* informations. The technique is the same : we pack adjacents or more distant
* tokens together to form candidates.
* What's different is the distance used to define what is adjacent and more
* distant. With syntactic informations, the distance is the distance between
* nodes in a tree, whereas without, it's the distance between tokens
* in the sentence. See examples if it is unclear.
*
*/
template<class T>
class CandidateExtractor : public CandidateFilter<T> {
	private:
		typedef std::vector<std::vector<Token *> > token_arrays;

		// Permutations
		std::vector<std::vector<std::vector<std::vector<int> > > > permutations;
		std::vector<std::vector<int> > getPermutations(int n, int m);

	protected:
		// general parameters
		int nFactors;
		int surfMin;
		int surfMax;
		bool extractDependency;

		// sentence processing
		Token nullToken;
		std::vector<Token *> sentence;
		std::vector<Tree<Token *>*> trees;
		void buildDepTree(std::vector<Tree<Token *>* > &trees, Token *token);
		token_arrays scanDepTree(int n, Tree<Token *> *cur);
		token_arrays scanSurfTree(int n, Tree<Token *> *cur, int depth);
		typedef std::function < void (std::vector<WordType *>, std::vector<int>,
									  int, WordType *, WordType *) > cb_candidate;
		void computeDepCandidates(cb_candidate);
		void computeSurfCandidates(cb_candidate);
		void computeCandidate(std::vector<Token *> tokens, bool isId, cb_candidate cb);
		using CandidateFilter<T>::addCandidate;
		void addCandidate(std::vector<Token *> tokens, bool isId);
		static token_arrays concat(token_arrays prefix, token_arrays bloc,
								   unsigned int order);

	public:
		CandidateExtractor(int n, int nFactors, int surfMin, int surfMax,
						   bool dependency);
		~CandidateExtractor();
		void addToken(std::string s);
		void computeCandidatesSentence();
};
}

#include "candidate_extractor.tpp"

#endif
