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

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <ostream>

#include "word_type.h"


namespace mwer{
/**
* @brief A token
*
* A token can be seen as an array of factors. The first factor is
* usually a word. The other factors are obtained by morphological and/or
* syntactic annotation. The software can use several factors, such as :
* @ref FORM, @ref LEMMA, @ref TAG, @ref ID, @ref PARENT_ID, @ref FUNCTION.
* The position of each factors in the token can be changed using those
* constants.
*
* It is possible that a token contains other factors which will just be
* ignored by the software.
*/
class Token {
	private:
		std::vector<std::string> factors;
		WordType *type;
		int id;
		int parentId;
	public:
		Token(int nFactors);
		Token(int nFactors, std::string tok);
		Token(int nFactors, std::string tok, int id);
		~Token();

		std::string getFactor(int n);
		WordType *getWordType();
		void setWordType(WordType *);

		int getParentId();
		int getId();
		friend std::ostream &operator<<(std::ostream &, Token &);

		static bool idIsInferior(Token *, Token *);
};

std::ostream &operator<<(std::ostream &os, Token &t);
}

#endif
