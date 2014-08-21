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

#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include <string>
#include <vector>

#include "parser.h"
#include "shared.h"
#include "candidate_filter.h"
#include "candidate.h"
#include "context_candidate.h"

using namespace mwer;
using namespace std;

int main(int argc, char *argv[])
{
	string inputFile;
	string inputFilesList;
	string outputFile;
	string lemmaFilter;
	string tagFilter;
	int minFreqFilter = -1;
	int maxFreqFilter = -1;
	int n = -1;
	bool filterOutFlag = false;
	opterr = 0;
	int cmdline;

	while ( (cmdline = getopt(argc, argv, "f:hi:l:n:o:rt:")) != -1) {
		switch (cmdline) {
			case 'f':
				getRange(string(optarg), minFreqFilter, maxFreqFilter);
				break;

			case 'h':
				cout << "filter_candidates : Filters MWE candidates." << endl;
				cout << "filter_candidates -n {2,3,4} -i CANDIDATE_LIST -o FILTERED_LIST" << endl;
				cout << "[-l regexp1:...:regexpn] [-t regexp1:...:regexpn] [-f min-max] [-r]" <<
					 endl;
				cout << "Mandatory : " << endl;
				cout << "  -n : 2,3 or 4" << endl;
				cout << "  -i : input candidate list file" << endl;
				cout << "  -o : output filtered candidate list file" << endl;
				cout << "Optional : " << endl;
				cout << "  -r : If set, reject matching candidates (applies to all filter)" << endl;
				cout << "       If not set, reject non matching candidates" << endl;
				cout << "  -f min-max : frequency filter" << endl;
				cout << "  -l regexp1:...:regexpn : regex filter for lemmas" << endl;
				cout << "  -t regexp1:...:regexpn : regex filter for tags" << endl;

				exit(0);

			case 'i':
				inputFile = optarg;
				break;

			case 'l':
				lemmaFilter = optarg;
				break;

			case 'n':
				n = atoi(optarg);
				break;

			case 'o':
				outputFile = optarg;
				break;

			case 'r':
				filterOutFlag = true;
				break;

			case 't':
				tagFilter = optarg;
				break;

			case '?':
				cout << "Error: unrecognized option -" << (char) optopt <<
					 " OR missing argument" << endl;
				return 1;

			default:
				cout << "??" << endl;
		}
	}

	if (inputFile.empty() && inputFilesList.empty()) {
		cerr << "Error: no file(s) to read ... use -i" << endl;
		return 1;
	}

	if (outputFile.empty()) {
		cerr << "Error: no filename for the output... use -o" << endl;
		return 1;
	}

	if (n == -1) {
		cerr << "Error: you have to provide parameter n... use -n" << endl;
		return 1;
	}

	if (n < 2 || n > 4) {
		cerr << "Error: n must be between 2 and 4" << std::endl;
		return 1;
	}

	cout << "Filtering " << n << "-grams candidates" << endl;
	cout << "Output file : " << outputFile << endl;
	Parser p(inputFile, SEP_WORDS, SEP_FACTORS, SEP_SECTIONS);
	int nFactors = p.getNumberOfFactors();
	CandidateFilter<ContextCandidate> cf(n);
	string s;
	vector<int> parentIds;
	parentIds.reserve(n);
	vector<WordType *> types(n);
	while (!p.endOfFile()) {
		int i = 0;
		vector<string> section = p.getNextSection();

		for (auto & s : section) {
			vector<string> v = split(s, SEP_FACTORS);

			if (nFactors > TAG_C) {
				types[i] = cf.addWordType(v[FORM_OR_LEMMA_C], v[TAG_C]);
			} else {
				types[i] = cf.addWordType(v[FORM_OR_LEMMA_C]);
			}

			if (nFactors >= PARENT_ID_C) {
				parentIds.push_back(atoi(v[PARENT_ID_C].c_str()));
			}

			++i;
		}

		section = p.getNextSection();
		s = section[0];
		cf.addCandidate(types, parentIds, atoi(s.c_str()));

		parentIds.clear();
		p.goToNextLine();
	}

	if (filterOutFlag) {
		cout << "Filtering OUT (removing) all the filtered candidates" << endl;
	} else {
		cout << "Filtering IN (keeping) all the filtered candidates" << endl;
	}

	if (nFactors > FORM_OR_LEMMA_C && !lemmaFilter.empty()) {
		cout << "Applying the lemma filter : " << lemmaFilter << endl;
		cf.regexpFilter(LEMMA, lemmaFilter, filterOutFlag);
	}

	if (nFactors > TAG_C && !tagFilter.empty()) {
		cout << "Applying the tag filter : " << tagFilter << endl;
		cf.regexpFilter(TAG, tagFilter, filterOutFlag);
	}

	if (minFreqFilter >= 0 && maxFreqFilter >= minFreqFilter) {
		cout << "Applying the frequency filter within the range ";
		cout << minFreqFilter << "-" << maxFreqFilter << endl;
		cf.frequencyFilter(minFreqFilter, maxFreqFilter, filterOutFlag);
	}

	cf.writeToFile(outputFile);
	return 0;
}

