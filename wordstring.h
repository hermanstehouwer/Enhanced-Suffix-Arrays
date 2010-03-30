// Part of the suffix array package by Herman Stehouwer
// Licenced under the GPLv3, see the LICENCE file.
//
// Copyright (C) 2010 Herman Stehouwer
// //
// // This program is free software: you can redistribute it and/or modify
// // it under the terms of the GNU General Public License as published by
// // the Free Software Foundation, either version 3 of the License, or
// // (at your option) any later version.
// //
// // This program is distributed in the hope that it will be useful,
// // but WITHOUT ANY WARRANTY; without even the implied warranty of
// // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// // GNU General Public License for more details.
// //
// // You should have received a copy of the GNU General Public License
// // along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <limits>


namespace std
{

class wordstring: public vector<string> {
  public:
  wordstring() {}
  wordstring(const_iterator b, const_iterator e):vector<string>(b, e) {}


  size_type
    length( ) const { return size(); }

  wordstring
    substr(size_type b, size_type n) const { return wordstring(begin()+b, begin()+b+n); }
};

 istream& operator>>(istream& in, wordstring& vec) // output
 {
	string curr;
	char c;
	int count = 0;
	while(in.good())
	{
		c = in.get();
		if('!' <= c and c <= '~')
		{
			curr.push_back(c);
		}
		else
		{
			if(curr.size() > 0)
			{	++count;
				vec.push_back(curr);
			}
			curr = string();
		}
	}
	if(curr.size() > 0)
	{
		vec.push_back(curr);
		++count;
	}
	cerr << "INTSTRING: READ IN " << count << " words" << endl;
	return in;
}

	ostream& operator<<(ostream& out, const wordstring& ws)
	{
		for (vector<string>::const_iterator i=ws.begin(); i!=ws.end(); i++) {
      out << *i << " ";
    }
		return out;
	}


}
