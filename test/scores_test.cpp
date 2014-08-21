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

#include "score_calculator.h"

using namespace std;

void printResults(std::vector<float> res){
	cout<< "Scores : " << endl;
	for(auto& score: res){
		cout<< score <<" ";
	}
	cout<<endl;
}

int main(){
	ScoreCalculator sc(true, true, {1, 6, 23, 39, 41, 57, 58, 59, 62, 68, 75, 77, 81}, 1.0);

	// Before starting tests, we enter context informations
	sc.addType("a", 15, {"b", "c", "g", "i"}, {1, 2, 3, 4});
	sc.addType("b", 3, {"a","c", "g", "i"}, {3, 5, 2});
	sc.addType("c", 4, {"a", "b"}, {3,5});
	sc.addType("d", 5, {"e", "a", "b"}, {3, 2, 1});
	sc.addType("e", 7, {"d"}, {1});
	sc.addType("f", 1, {"d"}, {1});
	sc.addType("g", 7, {"a", "b", "j"}, {1, 5, 2});
	sc.addType("h", 2, {"d"}, {2});
	sc.addType("i", 1, {"a", "b"}, {1,1});
	sc.addType("j", 3, {"d"}, {3});

	// We create a candidate with the optional type infos which will be
	// needed for broad context measures	
	sc.newCandidate({"a", "b"});

	sc.addContingencyTable({19,3,5,2904});

	// Immediate context tests :
	// Note that there are 19 elements in the left context and 12 in the right
	// It happens
	sc.addToImmediateContext(ScoreCalculator::LEFT, {"a", "b", "c", "d", "e", "f"} , {3, 10, 1, 1, 2, 2});
	
	sc.addToImmediateContext(ScoreCalculator::RIGHT, {"g", "h", "i", "j"}, {1, 8, 1, 2});

	//Broad context tests :
	sc.addToBroadContext({"a","c", "g", "i"}, {1,1, 1, 3});	
	printResults(sc.compute());


	
}
