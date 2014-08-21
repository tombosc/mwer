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
#include <string>
#include <getopt.h>
#include <algorithm>
#include <fstream>
#include <utility>
#include <memory>

#include "score_calculator.h"
#include "parser.h"
#include "shared.h"

using namespace mwer;
using namespace std;

pair<vector<string>, vector<int> > parseContext(vector<string> v)
{
	vector<string> resName;
	resName.reserve(v.size());
	vector<int> resFreq;
	resFreq.reserve(v.size());

	for (auto & a : v) {
		if (!a.empty()) {
			vector<string> temp = splitPair(a, SEP_REGEXPS); // type:freq
			resName.push_back(temp[0]);
			resFreq.push_back(std::stoi(temp[1]));
		}
	}

	return make_pair(resName, resFreq);
}


int main(int argc, char *argv[])
{
	bool compressedOutput;
	string statisticsFile;
	string output;
	opterr = 0;
	float smoothingParam = 0.5;
	static struct option long_options[] = {
		// flags
		{"help",  no_argument, 0, 'h'},
		// parameters with argument
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"smoothing", optional_argument, 0, 's'},
		{0, 0, 0, 0}
	};
	int option_index;
	int cmdline;

	while ((cmdline = getopt_long(argc, argv, "hi:o:s:", long_options,
								  &option_index)) != -1) {
		switch (cmdline) {
			case 'i':
				statisticsFile = optarg;
				break;

			case 'h':
				cout << "compute_scores : Computes scores" << endl;
				cout << "compute_scores s1 [s2 ... sn] -i input_stat -o output_scores " << endl;
				cout << "[-s smoothing_parameter]" << endl;
				cout << "Mandatory : " << endl;
				cout << "  s1 [s2 ... sn] : scores to compute" << endl;
				cout << "  -i : input stat file" << endl;
				cout << "  -o : output score file" << endl;
				cout << "Optional : " << endl;
				cout << "  -s : smoothing parameter (default=" << smoothingParam << ")" << endl;

				return 0;

			case 'o':
				output = optarg;
				break;

			case 's':
				smoothingParam = std::stof(optarg);
				break;

			case '?':
				cout << "Error: unrecognized option -" << (char) optopt <<
					 " OR missing argument" << endl;
				return 1;

			default:
				break;
		}
	}

	std::vector<int> toCompute;

	for (int index = optind; index < argc; index++) {
		toCompute.push_back(std::stoi(argv[index]));
	}

	if (toCompute.empty()) {
		cerr << "Error: no score to compute... pass score numbers as non-option";
		cerr << " arguments" << endl;
		return 1;
	}

	if (statisticsFile.empty()) {
		cerr << "Error: no statistics filename provided... use -i" << endl;
		return 1;
	}

	if (output.empty()) {
		cerr << "Error: no score output filename provided... use -o" << endl;
		return 1;
	}

	cout << "Reading statistics file " << statisticsFile << endl;
	cout << "Output scores in " << output << endl;
	cout << "Computing scores : ";

	for (int score : toCompute) {
		cout << score << " ";
	}

	cout << endl;
	cout << "Smoothing parameter : " << smoothingParam << endl;
	Parser parser(statisticsFile, SEP_WORDS, SEP_FACTORS, SEP_SECTIONS);
	int nSections = parser.getNumberOfSections();
	bool immediateContext = false;
	bool broadContext = false;

	if (nSections == 3) {
		broadContext = true;
		immediateContext = true;
	} else if (nSections == 4) {
		immediateContext = true;
	}

	int maxScore = *std::max_element(toCompute.begin(), toCompute.end());

	if (maxScore > 55 && !immediateContext) {
		cerr << "Error: score #" << maxScore;
		cerr << " impossible to process without immediate context" << endl;
		exit(1);
	}

	if (maxScore > 60 && !broadContext) {
		cerr << "Error: score #" << maxScore;
		cerr << " impossible to process without broad context" << endl;
		exit(1);
	}

	ScoreCalculator sc(immediateContext, broadContext,
					   toCompute, smoothingParam);
	string s;
	std::vector<std::string> section, types;
	std::vector<int> contingencyTable(16); // 16 is the size for n = 4

	unique_ptr<ofstream> scoreFile(new ofstream(output));
	if (!*scoreFile) {
		cerr << "Error: opening file " << output << endl;
		exit(1);
	}
	unique_ptr<ostream> stream;

	compressedOutput = (getExtension(output) == ".gz");
	if (compressedOutput) {
		stream = getCompressedStream(*scoreFile);
	} else {
		stream = std::move(scoreFile);
	}

	while (!parser.endOfFile()) {
		types = parser.getNextSection();

		if (types.size() == 1) { // one type candidate with broad context
			int freq = std::stoi((parser.getNextSection())[0]);
			auto context = parseContext(parser.getNextSection());
			sc.addType(types[0], freq, context.first, context.second);
		} else { // normal candidate
			if (broadContext) {
				// We strip the candidate to keep only comparable factors :
				// form & lemma
				vector<string> strippedTypes(types);
				for (string & t : strippedTypes) {
					size_t first_sep, second_sep;
					first_sep = t.find(SEP_FACTORS, 0);

					if (first_sep != string::npos) {
						second_sep = t.find(SEP_FACTORS, first_sep + 1);

						if (second_sep != string::npos) {
							t.erase(t.begin() + second_sep, t.end());
						}
					}
				}

				sc.newCandidate(strippedTypes);
			} else {
				sc.newCandidate();
			}

			section = parser.getNextSection();
			std::transform(section.begin(), section.end(),
						   contingencyTable.begin(),
			[] (std::string & s) {
				return stoi(s);
			});
			sc.addContingencyTable(contingencyTable);

			if (immediateContext) {
				auto context = parseContext(parser.getNextSection());
				sc.addToImmediateContext(ScoreCalculator::LEFT,
										 context.first, context.second);
				context = parseContext(parser.getNextSection());
				sc.addToImmediateContext(ScoreCalculator::RIGHT,
										 context.first, context.second);
			}

			if (immediateContext) {
				auto context = parseContext(parser.getNextSection());
				sc.addToBroadContext(context.first, context.second);
			}

			auto scores = sc.compute();

			// Output in file

			for (auto it = types.begin(); it != types.end() - 1; ++it) {
				*stream << *it << SEP_WORDS;
			}

			*stream << types.back() << SEP_SECTIONS;

			for (auto s = scores.begin(); s != scores.end() - 1; ++s) {
				*stream << std::to_string(*s) << SEP_WORDS;
			}

			*stream << std::to_string(scores.back()) << endl;
		}

		parser.goToNextLine();
	}

	return 0;
}
