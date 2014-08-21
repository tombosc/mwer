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
#include <unistd.h>
#include <string>
#include <limits>
#include <getopt.h>

#include "parser.h"
#include "shared.h"
#include "statistic_extractor.h"
#include "context_candidate.h"

using namespace mwer;
using namespace std;

int main(int argc, char *argv[])
{
	string corpus;
	string candidatesList;
	string outputFile;
	string tagFilter;
	int n = 2;
	int dependencyFlag = -1;
	int minSurfaceDistance = -1;
	int maxSurfaceDistance = -1;
	int immediateFlag = 0;
	int broadFlag = 0;
	int adjacentFlag = -1;
	opterr = 0;
	static struct option long_options[] = {
		// flags
		{"surface",   no_argument, &dependencyFlag, 0},
		{"adjacent",   no_argument, &adjacentFlag, 1},
		{"dependency",   no_argument, &dependencyFlag, 1},
		{"immediate", no_argument, &immediateFlag, 1},
		{"broad", no_argument, &broadFlag, 1},
		{"help",  no_argument, 0, 'h'},
		{"tag-filter", required_argument, 0, 't'},
		// parameters with argument
		{"corpus",  required_argument, 0, 'c'},
		{"input", required_argument, 0, 'i'},
		{"n",    required_argument, 0, 'n'},
		{"output",    required_argument, 0, 'o'},
		{"distance-range", required_argument, 0, 'r'},
		{0, 0, 0, 0}
	};
	int option_index;
	int cmdline;

	while ((cmdline = getopt_long(argc, argv, "ac:dhi:n:o:r:st:", long_options,
								  &option_index)) != -1) {
		switch (cmdline) {
			case 'a':
				adjacentFlag = 1;
				break;
			case 'c':
				corpus = optarg;
				break;

			case 'd':
				dependencyFlag = 1;
				break;

			case 'i':
				candidatesList = optarg;
				break;

			case 'h':
				cout << "extract_statistics : Extracts MWE candidates' statistics." << endl;
				cout << "extract_statistics -n {2,3,4} -c CORPUS_FILE -o OUTPUT_FILE";
				cout << endl << " -i CANDIDATES_FILE {-d|-s} [-a] [-r dist_min-dist_max]" << endl;
				cout << "[--immediate] [--broad] [-t regexp1:...:regexpn]" << endl;
				cout << "Mandatory : " << endl;
				cout << "  -n : 2,3 or 4" << endl;
				cout << "  -c : input corpus file" << endl;
				cout << "  -o : output candidates file" << endl;
				cout << "  -i : input candidate list file" << endl;
				cout << "  -d, --dependency	: dependency extraction OR" <<endl;
				cout << "  -s, --surface : surface extraction" << endl;
				cout << "Optional : " << endl;
				cout << "  -a, --adjacent : extract candidates that are adjacent only" << endl;
				cout << "  -r min-max : distance filter (accept matchs)" << endl;
				cout << "  -t regexp1:...:regexpn : regex filter for tags of context (accept matchs)" << endl;
				cout << "  --immediate : process immediate context" << endl;
				cout << "  --broad : process broad context" << endl;
				return 0;

			case 'n':
				n = atoi(optarg);
				break;

			case 'o':
				outputFile = optarg;
				break;

			case 'r':
				getRange(string(optarg), minSurfaceDistance, maxSurfaceDistance);
				break;

			case 's':
				dependencyFlag = 0;
				break;

			case 't':
				tagFilter = optarg;
				break;

			case '?':
				cout << "Error: unrecognized option -" << (char) optopt <<
					 " OR missing argument" << endl;
				return 1;

			default:
				break;
		}
	}

	if (candidatesList.empty()) {
		cerr << "Error: no candidate list provided... use -i" << endl;
		return 1;
	}

	if (corpus.empty()) {
		cerr << "Error: no corpus provided... use -c" << endl;
		return 1;
	}

	if (outputFile.empty()) {
		cerr << "Error: no filename for the output... use -o" << endl;
		return 1;
	}

	if (n < 2 || n > 4) {
		cerr << "Error: n must be between 2 and 4" << endl;
		return 1;
	}

	if (dependencyFlag == -1) {
		cerr << "Error: Choose between syntactical (-d) or surface (-s) extraction" <<
			 endl;
		return 1;
	}

	if (adjacentFlag == 1) {
		minSurfaceDistance = n - 1;
		maxSurfaceDistance = n - 1;
	}

	if (minSurfaceDistance >= 1 && maxSurfaceDistance >= minSurfaceDistance) {
		cout << "Accepted range of distance between each words of a candidate : ";
		cout << minSurfaceDistance << "-" << maxSurfaceDistance << endl;
	} else if (minSurfaceDistance == -1 && maxSurfaceDistance == -1) {
		minSurfaceDistance = n - 1;
		maxSurfaceDistance = std::numeric_limits<int>::max();
	}

	if (immediateFlag) {
		cout << "Processing immediate (left & right) context" << endl;
	}

	if (broadFlag) {
		cout << "Processing broad context" << endl;
	}

	if (!tagFilter.empty()) {
		cout << "Filtering IN only tags matching " << tagFilter << endl;
	}

	cout << "Working on " << n << "-grams" << endl;
	cout << "Reading corpus : " << corpus << endl;
	cout << "Reading candidate list : " << candidatesList << endl;
	cout << "Output file : " << outputFile << endl;
	Parser candidatesParser(candidatesList, SEP_WORDS, SEP_FACTORS, SEP_SECTIONS);
	int nFactorsCandidates = candidatesParser.getNumberOfFactors();
	Parser textParser(corpus, SEP_WORDS, SEP_FACTORS);
	int nFactorsCorpus = textParser.getNumberOfFactors();
	StatisticExtractor se(n, nFactorsCorpus, minSurfaceDistance,
						  maxSurfaceDistance, (bool) dependencyFlag,
						  (bool) immediateFlag, (bool) broadFlag, tagFilter);
	string s;
	vector<int> parentIds;
	parentIds.reserve(n);
	vector<WordType *> types(n);
	while (!candidatesParser.endOfFile()) {
		int i = 0;
		vector<string> strTypes = candidatesParser.getNextSection();

		for (auto & s : strTypes) {
			vector<string> v = split(s, SEP_FACTORS);

			if (nFactorsCandidates > TAG_C) {
				types[i] = se.addWordType(v[FORM_OR_LEMMA_C], v[TAG_C]);
			} else {
				types[i] = se.addWordType(v[FORM_OR_LEMMA_C]);
			}

			if (nFactorsCandidates >= PARENT_ID_C) {
				parentIds.push_back(atoi(v[PARENT_ID_C].c_str()));
			}

			++i;
		}

		// we set all the frequencies to 0 so that when the text is read, they are
		// incremented to their real values
		se.addCandidate(types, parentIds, 0);

		parentIds.clear();
		candidatesParser.goToNextLine();
	}

	while (!textParser.endOfFile()) {
		for (int i = 0; i < textParser.getNumberOfTokens(); i++) {
			s = textParser.getNextToken();
			se.addToken(s);
		}

		se.updateStatistics();
		textParser.goToNextLine();
	}

	se.finish();
	se.writeToFile(outputFile);
	return 0;
}
