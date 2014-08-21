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

#include "candidate_extractor.h"
#include "candidate.h"
#include "word_type.h"

#include <iostream>

using namespace mwer;

int main(int argc, char* argv[]){
	if(argc != 5){
		std::cerr<<"Error: Too much or not enough parameters"<<std::endl;
		std::cerr<<"Use: extractor_test n surf_min surf_max bool_extract_dep"<<std::endl;
		return 1;
	}

	int n = atoi(argv[1]);
	int surfMin = atoi(argv[2]);
	int surfMax = atoi(argv[3]);
	bool extractDep = atoi(argv[4]);

	// In this example, the maximum dependency distance is 4.
	// There are 6 words so the maximum surface distance is 5.
	std::vector<std::string> s = {"first|first|A|1|0", "second|second|A|2|1",
		 "third|third|A|3|1", "fourth|fourth|A|4|2", "fifth|fifth|A|5|1", "sixth|sixth|A|6|4"};

	CandidateExtractor<Candidate> ce(n, 5, surfMin, surfMax, extractDep);

	ce.addToken(s[0]);
	ce.addToken(s[1]);
	ce.addToken(s[2]);
	ce.addToken(s[3]);
	ce.addToken(s[4]);
	ce.addToken(s[5]);
	ce.computeCandidatesSentence();
	ce.printCandidates();

	return 0;
}
