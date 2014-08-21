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

#include "token.h"

#include "shared.h"

#include <iostream>

namespace mwer{
using namespace std;


/**
* @brief Create an empty token.
*
* This can be useful to create the root node of a sentence.
*
* @param nFactors
*/
Token::Token(int nFactors) :
	factors(nFactors, "0")
{
}



/**
* @brief Create a token for surfacic extraction.
*
* @param nFactors Number of factors
* @param tok Token in the form of factors separated by @ref SEP_FACTORS
* @param id Position in the sentence
*/
Token::Token(int nFactors, std::string tok, int id) :
	factors(nFactors),
	id(id)
{
	factors = split(tok, SEP_FACTORS);

	if ((int) factors.size() != nFactors) {
		cerr << "Error: Added token " << tok << " with incorrect number of factors";
		cerr << std::endl;
	}
}



/**
* @brief Create a token for dependency extraction
*
* @param nFactors Number of factors
* @param tok Token in the form of factors separated by @ref SEP_FACTORS
*/
Token::Token(int nFactors, std::string tok) :
	id(0) //last parameter doesn't matter, will be overriden
{
	factors = split(tok, SEP_FACTORS);

	if ((int) factors.size() != nFactors) {
		cerr << "Error: Added token " << tok << " with incorrect number of factors";
		cerr << std::endl;
	}

	id = atoi(getFactor(ID).c_str());
	parentId = atoi(getFactor(PARENT_ID).c_str());
}



Token::~Token()
{
}



ostream &operator<<(ostream &os, Token &t)
{
	for (auto it = t.factors.begin(); it != t.factors.end(); ++it) {
		os << (*it);

		if (it != t.factors.end() - 1) {
			os << SEP_FACTORS;
		}
	}

	return os;
}



/**
* @return Factor numbered n
*/
string Token::getFactor(int n)
{
	return factors[n];
}



/**
* @return pointer on WordType corresponding to the token
*/
WordType *Token::getWordType()
{
	return type;
}



/**
* @brief Set the WordType associated to the token to t
*
* @param t Pointer to WordType
*/
void Token::setWordType(WordType *t)
{
	type = t;
}



/**
* @brief Returns the ID of the parent token
*
* This function makes sense only in dependency extraction.
* The returned value is meaningless in surfacic extraction.
*
* @return The ID of the parent token 
*/
int Token::getParentId()
{
	return parentId;
}



/**
* @brief
*
* @return The position of the token in the sentence
*/
int Token::getId()
{
	return id;
}



/**
* @brief Compare two tokens' ids
*
* @param t1
* @param t2
*
* @return true if t1's id is inferior to t2's id
*/
bool Token::idIsInferior(Token *t1, Token *t2)
{
	return (t1->getId() < t2->getId());
}
}
