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

#ifndef SHARED_H_
#define SHARED_H_

#include <cstdlib>
#include <string>
#include <vector>
#include <memory>

#define SEP_FACTORS '|'
#define SEP_FACTORS_DOUBLEQUOTE "|"
#define SEP_WORDS ' '
#define SEP_RANGE "-"
#define SEP_REGEXPS ':'
#define SEP_SECTIONS '\t'

/**
 * Constants defining the positions of each factor in a corpus file
 */
#define FORM 0
#define LEMMA 1
#define TAG 2
#define ID 3
#define PARENT_ID 4
#define FUNCTION 5

/**
 * Constants defining the positions of each factor in a candidate file
 */
#define FORM_OR_LEMMA_C 0
#define TAG_C 1
#define ID_C 2
#define PARENT_ID_C 3

#define BOOST_BIND_NO_PLACEHOLDERS

namespace mwer{
std::unique_ptr<std::istream> getUncompressedStream(std::ifstream& file);

std::unique_ptr<std::ostream> getCompressedStream(std::ofstream& file);

std::string getExtension(std::string f);

void getRange(std::string s, int &min, int &max);

std::vector<std::string> split(std::string s, char sep);

std::vector<std::string> splitPair(std::string s, char sep);

bool contains(std::string s, std::string regexp);
}

#endif
