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

#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <istream>
#include <vector>
#include <memory>

namespace mwer{
/**
* @brief A parser for text file
*
* This class is used to parse a text file containing tokens, candidates, statistics...
*
*/
class Parser {
	private:
		// General file informations
		std::string filename;
		char sep_tokens;
		char sep_factors;
		char sep_sections;
		bool compressed;
		std::shared_ptr<std::ifstream> rawFile;
		std::shared_ptr<std::istream> file; 
		int nFactors;
		bool eof;

		// Current sentence informations
		std::string currentLine;
		int nSepWords;
		int nSections;
		int offsetToken;
		int offsetSections;
		std::vector<std::string> sections;
	public:
		Parser(std::string filename, char sep_t, char sep_f, char sep_s= '\0');
		~Parser();

		void goToNextLine();
		std::string getNextToken();
		std::vector<std::string> getNextSection();

		bool endOfFile() const;
		int getNumberOfTokens() const;
		int getNumberOfFactors() const;
		int getNumberOfSections() const;
};
}

#endif
