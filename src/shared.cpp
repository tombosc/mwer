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

#include "shared.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <boost/algorithm/string/split.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

#define BOOST_IOSTREAMS_USE_DEPRECATED

namespace mwer{
namespace io = boost::iostreams;

/**
* @brief Retrieve file extension if it exists
*
* @param f Path or filename
*
* @return File extension if it exists, or empty string
*/
std::string getExtension(std::string f)
{
	return boost::filesystem::path(f).extension().string();
}



/**
* @brief Get an input stream out of a gzip'ed file
*
* @param file filename of a gzip'ed file
*
* @return uncompressed stream. Warning : some functions
* are not available within the object... (ex. : seekg)
*/
std::unique_ptr<std::istream> getUncompressedStream(std::ifstream& file)
{
	typedef io::filtering_istream filter;
	std::unique_ptr<filter> in = std::unique_ptr<filter> (new filter());
	in->push(io::gzip_decompressor());
	in->push(file);
	return std::move(in);
}



/**
* @brief Get an output stream to a compressed (gzip) file
*
* @param file filename of a future gzip file
*/
std::unique_ptr<std::ostream> getCompressedStream(std::ofstream& file)
{
	typedef io::filtering_ostream filter;
	std::unique_ptr<filter> out = std::unique_ptr<filter> (new filter());
	out->push(io::gzip_compressor());
	out->push(file);
	return std::move(out);
}



/**
 * @brief Extract a range (2 integers) from a string
 *
 * If only one value (s == "i"), max will be given the maximum integer value
 *
 * @param s string containing to integers separated by @ref SEP_RANGE
 * @param min reference to the min value
 * @param max reference to the max value
 */
void getRange(std::string s, int &min, int &max)
{
	unsigned int pos = s.find(SEP_RANGE);

	if (pos == std::string::npos) {
		min = atoi(s.c_str());
		max = std::numeric_limits<int>::max();
	} else {
		min = atoi((s.substr(0, pos)).c_str());
		max = atoi((s.substr(pos + 1)).c_str());
	}
}


/**
 * @brief Split a string into several substrings
 *
 * If two separators are adjacent, it will generate an empty item in the vector
 *
 * @param s string to split
 * @param sep separator
 *
 * @return vector of substrings
 */
std::vector<std::string> split(std::string s, char sep)
{
	std::vector<std::string> res;
	auto equalSep = [sep](char c) {
		return c == sep;
	};
	boost::split(res, s, equalSep, boost::token_compress_off);
	return res;
}



/**
* @brief Split a string in 2 elements.
*
* If the separator appears several time, the string will be split on the last
* separator.
*
* @param s string to split
* @param sep separator
*
* @return vector (size 2) of substrings
*/
std::vector<std::string> splitPair(std::string s, char sep)
{
	std::vector<std::string> res(2);
	auto found = s.find_last_of(sep);
	res[0] = s.substr(0, found);
	res[1] = s.substr(found + 1);
	return res;
}



/**
 * @brief Matches a string with a regular expression
 *
 * @param s string
 * @param regexp regular expression
 *
 * @return true if s matches regexp
 */
bool contains(std::string s, std::string regexp)
{
	boost::regex e(regexp);
	return boost::regex_match(s, e);
}
}
