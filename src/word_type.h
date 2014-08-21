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

#ifndef WORDTYPE_H_
#define WORDTYPE_H_

#include <string>
#include <vector>
#include <functional>

namespace mwer{
/**
* @brief A general characterization of @ref Token
*
* A word type can be either : a word, a lemma or a lemma with a morphological
* tag. It defines in a general way the type of a @ref Token.
*/
class WordType {
	private:
		std::string formOrLemma;
		std::string tag;

	public:
		WordType(std::string formOrLemma);
		WordType(std::string formOrLemma, std::string tag);
		virtual ~WordType();

		std::string &getFormOrLemma();
		std::string &getTag();

		bool operator< (const WordType &wt) const;

		friend struct WordTypeHash;

		friend struct WordTypeEq;

		friend std::ostream &operator<<(std::ostream &, WordType &);

		friend bool operator==(const WordType &t1, const WordType &t2);
};

std::ostream &operator<<(std::ostream &os, WordType &t);

bool operator==(const WordType &t1, const WordType &t2);

/**
* @brief Hash functor for @ref WordType*
*/
struct WordTypeHash {
	std::hash<std::string> str_hash;
	size_t operator()(const WordType *t) const;
};

/**
* @brief Equality test functor for @ref WordType*
*/
struct WordTypeEq {
	bool operator()(const WordType *t1, const WordType *t2) const;
};
}

#endif
