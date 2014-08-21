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

#include "shared_test.h"
#include "shared.h"
#include <iostream>


using namespace std;

int main(){
	cout<<"getRange :"<<endl;

	int min, max;
	string s = "1-2";
	getRange(s, min, max);
	cout<<s<<" : between "<<min<<" and "<<max<<endl;
	s = "3-90";
	getRange(s, min, max);
	cout<<s<<" : between "<<min<<" and "<<max<<endl;
	s = "9";
	getRange(s, min, max);
	cout<<s<<" : between "<<min<<" and "<<max<<endl;
	
	cout<<"splitFactors :"<<endl;
	vector<string> v(8);

	s = "black|JJ|1|2|Atr";
	v = split(s, 1, SEP_FACTORS);
	cout<<"Extracting 1 factor : "<<v[0]<<endl;

	v = split(s, 3, SEP_FACTORS);
	cout<<"Extracting 3 factors : "<<v[0]<<", "<<v[1]<<", "<<v[2]<<endl;

	v = split(s, 5, SEP_FACTORS);
	cout<<"Extracting 5 factors : "<<v[0]<<", "<<v[1]<<", "<<v[2]<<", "<<v[3]<<", "<<v[4]<<endl;

	v = split(s, 20, SEP_FACTORS); // we set the max too high
	cout<<"Extracting all the factors : "<<v[0]<<", "<<v[1]<<", "<<v[2]<<", "<<v[3]<<", "<<v[4]<<endl;
	cout<<"checking if the 6th & 7th element are empty : "<<v[5]<<", "<<v[6]<<endl;
	return 0;
}
