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

#include "parser.h"
#include "shared.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#define DEBUG_PARSER 0

#if DEBUG_PARSER
#define TRACE(args) cout<<args<<endl
#else
#define TRACE(args)
#endif

namespace mwer{
using namespace std;

/**
* @brief Initialize parser & read the first line.
*
* If there is an error while opening the file, the program will stop.
*
* @param filename Path to the file to open
* @param sep_t Character separating tokens from each other
* @param sep_f Character separating factors from each other
* @param sep_s Optional : character separating sections from each other
*
*/
Parser::Parser(string filename, char sep_t, char sep_f, char sep_s) :
	filename(filename),
	sep_tokens(sep_t),
	sep_factors(sep_f),
	sep_sections(sep_s),
	eof(false)
{
	rawFile = make_shared<ifstream>(filename.c_str());
	compressed = (getExtension(filename) == ".gz");

	if (!*rawFile) {
		cerr << "Error: opening file " << filename << endl;
		exit(1);
	}

	if (compressed) {
		file = getUncompressedStream(*rawFile);
	} else {
		file = std::move(rawFile);	
	}

	// Calculate number of factors which will remain constant
	goToNextLine();
	if (!eof) {
		string firstWord = currentLine.substr(0, currentLine.find(sep_tokens));
		TRACE("First Word : " << firstWord);
		nFactors = count(firstWord.begin(), firstWord.end(), sep_factors) + 1;
		TRACE("Number of factors per token : " << nFactors);

		if (sep_sections != '\0') {
			nSections = count(currentLine.begin(), currentLine.end(),
								sep_sections) + 1;
		} else {
			nSections = 0;
		}
	}
}



Parser::~Parser()
{
}



/**
* @brief
*
* Go to the next readable sentence. After each call, you have to check
* if it's not the end of the file.
*/
void Parser::goToNextLine()
{
	if (getline(*file, currentLine)) {
		nSepWords = count(currentLine.begin(), currentLine.end(), sep_tokens) + 1;
		offsetToken = 0;
		offsetSections = 0;
		sections = split(currentLine, sep_sections);
	} else {
		eof = true;
	}
}



/**
* @brief
*
* @return the next unread token
*/
string Parser::getNextToken()
{
	int nextSep = currentLine.find(sep_tokens, offsetToken);
	string s = currentLine.substr(offsetToken, nextSep - offsetToken);
	offsetToken = nextSep + 1;
	TRACE("nextSep : " << nextSep);
	TRACE("TOKEN retourné : " << s << " offset : " << offsetToken);
	return s;
}



/**
* @brief
*
* @return a vector of string containing the next unread section
*/
vector<string> Parser::getNextSection()
{
	return split(sections[offsetSections++], sep_tokens);
}


/**
* @brief
*
* @return if there are no more line to read, true
*
*/
bool Parser::endOfFile() const
{
	return eof;
}



/**
* @brief
*
* @return the number of tokens of the current line
*/
int Parser::getNumberOfTokens() const
{
	return nSepWords;
}



/**
* @brief
*
* @return the number of factors of the first token of the first line
*/
int Parser::getNumberOfFactors() const
{
	return nFactors;
}



/**
* @brief
*
* @return the number of factors of the first token of the first line
*/
int Parser::getNumberOfSections() const
{
	return nSections;
}
}
