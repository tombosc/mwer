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

#include "word_type.h"

#include "shared.h"

#include <functional>
#include <tuple>

namespace mwer{
using namespace std;

WordType::WordType(std::string s) :
	formOrLemma(s)
{
}



WordType::WordType(std::string s, std::string t) :
	formOrLemma(s),
	tag(t)
{
}


WordType::~WordType()
{
}



ostream &operator<<(ostream &os, WordType &t)
{
	os << t.formOrLemma;

	if (!t.tag.empty()) {
		string sep(SEP_FACTORS_DOUBLEQUOTE);
		os << sep << t.tag;
	}

	return os;
}



string &WordType::getFormOrLemma()
{
	return formOrLemma;
}



string &WordType::getTag()
{
	return tag;
}



size_t WordTypeHash::operator()(const WordType *t) const
{
	return str_hash(t->formOrLemma);
}



bool WordType::operator< (const WordType &wt) const
{
	return std::tie(formOrLemma, tag) < std::tie(wt.formOrLemma, wt.tag);
}



bool operator==(const WordType &t1, const WordType &t2)
{
	return (t1.formOrLemma == t2.formOrLemma) &&
		   (t1.tag == t2.tag);
}



bool WordTypeEq::operator()(const WordType *t1, const WordType *t2) const
{
	return *t1 == *t2;
}
}
