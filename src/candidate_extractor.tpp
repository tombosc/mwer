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

#include <map>
#include <algorithm>
#include <functional>

namespace mwer{
using namespace std;
using namespace std::placeholders;

/**
* @brief
*
* @param n Number of word types contained in a MWE candidate
* @param nFactors Number of factors per token
* @param surfMin Minimal distance of the two furthest tokens in a candidate
* @param surfMax Maximal distance of the two furthest tokens in a candidate
* @param dependency If true, the extractor will search for candidates using
* syntaxic informations (that have to be provided in this case).
* if false, the extractor will search for candidates based on the position
* of the tokens in the sentence
*/
template<class T>
CandidateExtractor<T>::CandidateExtractor(int n, int nFactors,
										  int surfMin, int surfMax,
										  bool dependency) :
	CandidateFilter<T>(n),
	permutations(n + 1),
	nFactors(nFactors),
	surfMin(surfMin),
	surfMax(surfMax),
	extractDependency(dependency),
	nullToken(nFactors)
{
	this->sentence.reserve(MAX_WORDS_PER_SENTENCE);
	this->sentence.push_back(&nullToken);
	trees.reserve(MAX_WORDS_PER_SENTENCE);
}



template<class T>
CandidateExtractor<T>::~CandidateExtractor()
{
}



/**
* @brief Add a token in the current sentence
*
* @param tokenStr string containing the token in text representation :
* several nFactors separated by the character SEP_FACTORS (shared.h)
*/
template<class T>
void CandidateExtractor<T>::addToken(string tokenStr)
{
	// First, we create a token
	Token *token;

	if (nFactors >= PARENT_ID) {
		token = new Token(nFactors, tokenStr);
	} else {
		token = new Token(nFactors, tokenStr, sentence.size());
	}

	// Then, we create the corresponding word type
	WordType *type;

	//Example (f=factor) : f1|..|fi|form|fi+2|..|fj|lemma|fj+2|..|fk|TAG|fk+2|..
	if (nFactors > TAG) {
		type = CandidateFilter<T>::addWordType(token->getFactor(LEMMA),
											   token->getFactor(TAG));
	} else if (nFactors > LEMMA) {
		type = CandidateFilter<T>::addWordType(token->getFactor(LEMMA),
											   std::string());
	} else {
	type = CandidateFilter<T>::addWordType(token->getFactor(FORM),
											   std::string());
	}

	// We associate the type to the token
	token->setWordType(type);
	this->sentence.push_back(token);
	TRACE("Added token " << token->getFactor(FORM) << " to sentence");
}



/**
* @brief Compute dependency candidates
*
* First, it builds a tree using the factors id and parent id which describe
* a hierarchical syntactic relationship between tokens.
* Then it scans the tree and add every candidate that have a right distance.
*
* @param f Callback function to be executed on every candidate
*/
template<class T>
void CandidateExtractor<T>::computeDepCandidates(cb_candidate f)
{
	int size = this->sentence.size();
	token_arrays c;
	// Initialising the root
	Tree<Token *> *root = new Tree<Token *>(&nullToken);
	trees.resize(size, 0);
	trees[0] = root;

	for (int i = 1; i < size; i++) {
		if (trees[i] == 0) {
			buildDepTree(trees, this->sentence[i]);
		}
	}

	for (auto it = trees.begin() + 1; it != trees.end(); ++it) {
		c = scanDepTree(CandidateFilter<T>::n, *it);

		for (auto it = c.begin(); it != c.end(); ++it) {
			if ((int) it->size() == CandidateFilter<T>::n) {
				this->computeCandidate(*it, true, f);
			}
		}
	}

	// clear the tree
	for (auto it = trees.begin(); it != trees.end(); ++it) {
		delete *it;
	}

	trees.clear();
}



/**
* @brief Compute surface candidates
*
* First, it builds a chain (graph theory) where nodes are tokens.
* Then it scans the tree and add every candidate that have a right distance.
*
* @param f Callback function to be executed on every candidate
*/
template <class T>
void CandidateExtractor<T>::computeSurfCandidates(cb_candidate f)
{
	int size = this->sentence.size();
	token_arrays c;
	Tree<Token *> *root = new Tree<Token *>(&nullToken);
	trees.resize(size, 0);
	trees[0] = root;

	// Build a chain to compute candidates using the same
	// algorithm as for dependency bigrams
	// a chain is also a tree, so the algorithm works fine

	for (int i = 1; i < size; i++) {
		trees[i] = new Tree<Token *>(this->sentence[i]);
		trees[i]->linkWithFather(trees[i - 1]);
	}

	// root has only one child here
	c = scanSurfTree(CandidateFilter<T>::n, *root->childrenBegin(), 0);

	for (auto it = c.begin(); it != c.end(); ++it) {
		this->computeCandidate(*it, false, f);
	}

	// clear the tree
	for (auto it = trees.begin(); it != trees.end(); ++it) {
		delete *it;
	}

	trees.clear();
}



/**
* @brief This function must be called once that all the tokens of a sentence
* have been added with addToken. All the entered tokens will be used to
* compute all the candidates according to the parameters entered in the
* constructor.
*/
template<class T>
void CandidateExtractor<T>::computeCandidatesSentence()
{
	cb_candidate f = std::bind(
						 &CandidateFilter<T>::addCandidate,
						 this, _1, _2, _3);

	if (extractDependency) {
		computeDepCandidates(f);
	} else if ((int) this->sentence.size() > CandidateFilter<T>::n) {
		// sentence must be long enough to process surface candidates
		computeSurfCandidates(f);
	}

	// clear the sentences informations
	for (auto it = this->sentence.begin() + 1;
			it != this->sentence.end(); ++it) {
		delete *it;
	}

	this->sentence.clear();
	this->sentence.push_back(&nullToken);
}



/**
 * @brief Returns an array with all the possible permutations of m digits
 * summing to n
 *
 * example : getPermutations(3, 2) returns : {{2, 1}, {1, 2}}
 *
 * @param n
 * @param m
 *
 * @return 
 */
template<class T>
vector<vector<int> > CandidateExtractor<T>::getPermutations(int n, int m)
{
	if ((int) permutations[n].size() < m + 1) {
		permutations[n].resize(m + 1);
	}

	if (permutations[n][m].size() != 0) {
		return permutations[n][m];
	}

	if (m == 1) {
		this->permutations[n][m].push_back({{n}});
		return permutations[n][m];
	}

	vector< vector<int> > v;

	for (int j = 0; j <= n; j++) {
		vector<vector<int> > res = getPermutations(n - j, m - 1);

		for (unsigned int k = 0; k < res.size(); k++) {
			res[k].push_back(j);
			v.push_back(res[k]);
		}
	}

	permutations[n][m] = v;
	return permutations[n][m];
}



/**
 * @brief Apply f on the candidate if the distance is correct
 * (using surfMin, surfMax) 
 *
 * @param tokens Candidate in the form of tokens
 * @param isId if True, it means that tokens are not sorted by id
 * @param f Callback function to apply
 */
template<class T>
void CandidateExtractor<T>::computeCandidate(vector<Token *> tokens,
											 bool isId,
											 cb_candidate f)
{
	vector<WordType *> types(tokens.size());
	vector<int> pids;
	map<int, int> mappingIds;

	if (isId) {
		std::sort(tokens.begin(), tokens.end(), Token::idIsInferior);
	}

	int maxDistance = (tokens.back())->getId() - (tokens.front())->getId();

	if (maxDistance > surfMax || maxDistance < surfMin) {
		return;
	}

	mappingIds[0] = 0;
	int counter = 0;

	for (auto it = tokens.begin(); it != tokens.end(); ++it) {
		mappingIds[(*it)->getId()] = counter + 1;
		types[counter] = (*it)->getWordType();
		TRACE("map[" << (*it)->getId() << "] = " << counter + 1);
		++counter;
	}

	if (isId) {
		pids.resize(tokens.size());
		counter = 0;

		for (auto it = tokens.begin(); it != tokens.end(); ++it) {
			pids[counter] = mappingIds[(*it)->getParentId()];
			TRACE("pids[" << counter << "] = " << pids[counter]);
			++counter;
		}
	}

	WordType *typePrev, *typeNext;
	int idFirst = tokens.front()->getId();
	int idLast = tokens.back()->getId();

	if (idFirst == 1) {
		typePrev = 0;
	} else {
		typePrev = sentence[idFirst - 1]->getWordType();
	}

	if (idLast == (int) sentence.size() - 1) {
		typeNext = 0;
	} else {
		typeNext = sentence[idLast + 1]->getWordType();
	}

	f(types, pids, 1, typePrev, typeNext);
}



/**
 * @brief Extract a token_arrays of all possible candidates in a dependency tree
 *
 * @param order Number of types in a candidate : should be n
 * This parameter exists because it is a recursive function.
 * @param t root of the tree of tokens
 *
 * @return token_arrays of all possible candidates
 */
template<class T>
typename CandidateExtractor<T>::token_arrays
CandidateExtractor<T>::scanDepTree(
	int order, Tree<Token *> *t)
{
	int nChildren = t->numberOfChildren();
	Token *token = t->getElement();

	// case 1 : leaf
	if (nChildren == 0) {
		// if we needed only 1 type, we add it
		// we therefore output only 1 candidate
		if (order == 1) {
			token_arrays c2(1);
			c2[0].push_back(token);
			return c2;
		}

		// if there was more types needed, we can't provide enough so we stop
		return token_arrays();
	}

	// case 2 : has children : recurse
	token_arrays totalPermutations;
	// compute all permutations that includes the current node
	// we are therefore looking for (order - 1) types among the children
	// in every possible ways
	auto combi = getPermutations(order - 1, nChildren);
	// we create an array result containing only the current node
	token_arrays singleton(1);
	singleton[0].push_back(token);

	// For each possible permutation,
	for (unsigned int i = 0; i < combi.size(); i++) {
		int j = 0; // first child index
		token_arrays temp = singleton;
		// set the current length of the permutation to 1 (singleton)
		int permutationLength = 1;

		// we will now generate all the token_arrays containing this singleton
		for (auto child = t->childrenBegin(); child != t->childrenEnd(); ++child) {
			if (combi[i][j] != 0) {
				auto res = scanDepTree(combi[i][j], *child);
				permutationLength += combi[i][j];
				temp = concat(temp, res, permutationLength);
			}

			j++;
		}

		totalPermutations.insert(totalPermutations.end(), temp.begin(), temp.end());
	}

	return totalPermutations;
}



/**
 * @brief Extract a token_arrays of all possible candidates in a surface tree (chain)
 *
 * @param order Number of types in a candidate : should be n
 * This parameter exists because it is a recursive function.
 * @param t root of the tree of tokens
 * @param depth Should be 0 in the beginning of the recursion
 *
 * @return token_arrays of all possible candidates
 */

template<class T>
typename CandidateExtractor<T>::token_arrays
CandidateExtractor<T>::scanSurfTree(
	int order, Tree<Token *> *t, int depth)
{
	Token *token = t->getElement();

	// case 1 : leaf or surfMax reached
	if (order < 1 || t->numberOfChildren() == 0 || depth == surfMax) {
		// if we needed only 1 type, we add it
		// we therefore output only 1 candidate
		if (order == 1 && surfMin <= depth) {
			token_arrays c2(1);
			c2[0].push_back(token);
			return c2;
		}

		// if there was more types needed, we can't provide enough so we stop
		return token_arrays();
	}

	Tree<Token *> *child = *(t->childrenBegin());
	// case 2 : has children and distance max not reached
	token_arrays totalPermutations;
	// compute all possibilities without the current node
	token_arrays childRes;

	if (depth == 0) {
		childRes = scanSurfTree(order, child, 0);
	} else {
		childRes = scanSurfTree(order, child, depth + 1);
	}

	totalPermutations.insert(totalPermutations.end(), childRes.begin(),
							 childRes.end());
	// compute all possibilities with the current node
	token_arrays temp(1);
	temp[0].push_back(token);
	childRes = scanSurfTree(order - 1, child, depth + 1);
	temp = concat(temp, childRes, order);
	totalPermutations.insert(totalPermutations.end(), temp.begin(), temp.end());
	return totalPermutations;
}




/**
 * @brief Tool-function to concatenate 2 arrays in a specific way
 *
 * Prefix will be append to block and only lines of size order will be kept
 *
 * @param prefix prefix to append to block
 * @param block  
 * @param order line size
 *
 * @return concatenated array
 */
template<class T>
typename CandidateExtractor<T>::token_arrays CandidateExtractor<T>::concat(
	token_arrays prefix, token_arrays block, unsigned int order)
{
	if (prefix.size() == 0) {
		return block;
	} else if (block.size() == 0) {
		return prefix;
	}

	token_arrays res;

	for (auto it1 = prefix.begin(); it1 != prefix.end(); ++it1) {
		for (auto it2 = block.begin(); it2 != block.end(); ++it2) {
			if (order == it1->size() + it2->size()) {
				vector<Token *> temp = *it1;
				temp.insert(temp.end(), it2->begin(), it2->end());
				res.push_back(temp);
			}
		}
	}

	return res;
}




/**
 * @brief Build a dependency tree according to tokens' ids and parent ids
 *
 * @param trees vector of trees of tokens preallocated
 * @param token token to insert in the tree
 */
template<class T>
void CandidateExtractor<T>::buildDepTree(vector<Tree<Token *>* > &trees,
										 Token *token)
{
	int id = token->getId();
	int parentId = token->getParentId();
	Tree<Token *> *t = new Tree<Token *>(token);
	trees[id] = t;
	Tree<Token *> *parent = trees[parentId];

	if (parent == 0) {
		buildDepTree(trees, this->sentence[parentId]);
		parent = trees[parentId];
	}

	t->linkWithFather(parent);
}
}
