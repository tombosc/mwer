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
#include <getopt.h>
#include <vector>
#include <limits>

#include "parser.h"
#include "shared.h"
#include "candidate.h"
#include "candidate_extractor.h"

using namespace std;
using namespace mwer;

int main(int argc, char *argv[])
{
	string corpus;
	string outputFile;
	string lemmaFilter;
	string tagFilter;
	int minFreqFilter = -1;
	int maxFreqFilter = -1;
	int n = -1;
	int minSurfaceDistance = -1;
	int maxSurfaceDistance = -1;
	int dependencyFlag = -1;
	int adjacentFlag = 0;
	opterr = 0;
	static struct option long_options[] = {
		// flags
		{"surface",   no_argument, &dependencyFlag, 0},
		{"adjacent",   no_argument, &adjacentFlag, 1},
		{"dependency",   no_argument, &dependencyFlag, 1},
		{"help",  no_argument, 0, 'h'},
		// parameters with argument
		{"corpus",  required_argument, 0, 'c'},
		{"frequency-filter", required_argument, 0, 'f'},
		{"lemma-filter",  required_argument, 0, 'l'},
		{"n",    required_argument, 0, 'n'},
		{"output",    required_argument, 0, 'o'},
		{"distance-range", required_argument, 0, 'r'},
		{"tag-filter", required_argument, 0, 't'},
		{0, 0, 0, 0}
	};
	int option_index;
	int cmdline;

	while ( (cmdline = getopt_long(argc, argv, "ac:df:hl:n:o:r:st:",
								   long_options, &option_index)) != -1) {
		switch (cmdline) {
			case 0:
				if (long_options[option_index].flag != 0) {
					break;
				}

			case 'a':
				adjacentFlag = 1;
				break;

			case 'c':
				corpus = optarg;
				break;

			case 'd':
				if (dependencyFlag == -1) {
					dependencyFlag = 1;
				} else {
					cerr << "Error: you already selected dependency extraction (-d)";
					cerr << endl;
				}

				break;

			case 'f':
				getRange(string(optarg), minFreqFilter, maxFreqFilter);
				break;

			case 'h':
				cout << "extract_candidates : Extracts MWE candidates." << endl;
				cout << "extract_candidates -n {2,3,4} -c CORPUS_FILE -o OUTPUT_FILE";
				cout << endl << " {-d|-s} [-a] [-r dist_min-dist_max] [-f min-max]" << endl;
				cout << "[-l regexp1:...:regexpn] [-t regexp1:...:regexpn]" << endl;
				cout << "Mandatory : " << endl;
				cout << "  -n : 2,3 or 4" << endl;
				cout << "  -c : input corpus file" << endl;
				cout << "  -o : output candidates file" << endl;
				cout << "  -d, --dependency	: dependency extraction OR" <<endl;
				cout << "  -s, --surface : surface extraction" << endl;
				cout << "Optional : " << endl;
				cout << "  -a, --adjacent : extract candidates that are adjacent only" << endl;
				cout << "  -r min-max : distance filter (accept matchs)" << endl;
				cout << "  -f min-max : frequency filter (accept matchs)" << endl;
				cout << "  -l regexp1:...:regexpn : regex filter for lemmas (accept matchs)" << endl;
				cout << "  -t regexp1:...:regexpn : regex filter for tags (accept matchs)" << endl;
				exit(0);

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
				getRange(string(optarg), minSurfaceDistance, maxSurfaceDistance);
				break;

			case 's':
				if (dependencyFlag == -1) {
					dependencyFlag = 0;
				} else {
					cerr << "Error: you already selected surface extraction (-s)" << endl;
				}

				break;

			case 't':
				tagFilter = optarg;
				break;

			case '?':
				cout << "Error: unrecognized option -" << (char) optopt
					 << " OR missing argument" << endl;
				return 1;

			default:
				break;
		}
	}

	if (optind < argc) {
		printf ("non-option ARGV-elements: ");

		while (optind < argc) {
			printf ("%s ", argv[optind++]);
		}

		putchar ('\n');
	}

	if (corpus.empty()) {
		cerr << "Error: no corpus to read ... use -c file"
			 << endl;
		return 1;
	}

	if (outputFile.empty()) {
		cerr << "Error: no filename for the output... use -o" << endl;
		return 1;
	}

	if (n < 2 || n > 4) {
		cerr << "Error: n must be between 2 and 4" << std::endl;
		return 1;
	}

	if (dependencyFlag == -1) {
		cerr << "Error: Choose between syntactical (-d) or surface (-s) extraction"
			 << endl;
		return 1;
	}

	if (adjacentFlag == 1) {
		minSurfaceDistance = n - 1;
		maxSurfaceDistance = n - 1;
	}

	cout << "Looking for " << n << "-grams" << endl;

	if (minSurfaceDistance >= 1 && maxSurfaceDistance >= minSurfaceDistance) {
		cout << "Accepted range of distance between each words of a candidate : ";
		cout << minSurfaceDistance << "-" << maxSurfaceDistance << endl;
	}

	cout << "Output file : " << outputFile << endl;

	if (minSurfaceDistance == -1 && maxSurfaceDistance == -1) {
		minSurfaceDistance = n - 1;
		maxSurfaceDistance = std::numeric_limits<int>::max();
	}

	int nFactors = 1;
	CandidateExtractor<Candidate> *ce = 0;

	Parser p(corpus, SEP_WORDS, SEP_FACTORS);

	nFactors = p.getNumberOfFactors();
	ce = new CandidateExtractor<Candidate>(n, nFactors, 
										   minSurfaceDistance, maxSurfaceDistance,
										   (bool) dependencyFlag);

	cout << "Reading corpus : " << corpus << endl;

	if (dependencyFlag == 1 && nFactors <= PARENT_ID) {
		cerr << "Error: You chose dependency candidates extraction, but the ";
		cerr << "corpus doesn't have syntactical annotations." << endl;
		return 1;
	}

	string s;

	while (!p.endOfFile()) {
		for (int i = 0; i < p.getNumberOfTokens(); i++) {
			s = p.getNextToken();

			if (!s.empty()) {
				ce->addToken(s);
			}
		}

		ce->computeCandidatesSentence();
		p.goToNextLine();
	}

	if (nFactors > LEMMA && !lemmaFilter.empty()) {
		cout << "Applying the lemma filter : " << lemmaFilter << endl;
		ce->regexpFilter(LEMMA, lemmaFilter);
	}

	if (nFactors > TAG && !tagFilter.empty()) {
		cout << "Applying the tag filter : " << tagFilter << endl;
		ce->regexpFilter(TAG, tagFilter);
	}

	if (minFreqFilter >= 1 && maxFreqFilter >= minFreqFilter) {
		cout << "Applying the frequency filter within the range ";
		cout << minFreqFilter << "-" << maxFreqFilter << endl;
		ce->frequencyFilter(minFreqFilter, maxFreqFilter);
	}

	if (ce != 0) {
		ce->writeToFile(outputFile);
	}

	delete ce;
	return 0;
}

