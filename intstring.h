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

class intstring: public vector<int> {
  public:
  intstring() {}
  intstring(const_iterator b, const_iterator e):vector<int>(b, e) {}


  size_type
    length( ) const { return size(); }

  intstring
    substr(size_type b, size_type n) const { return intstring(begin()+b, begin()+b+n); }
};

 istream& operator>>(istream& in, intstring& vec) // output
 {
	string curr;
	char c;
	int count = 0;
	while(in.good())
	{
		c = in.get();
		if('0' <= c and c <= '9' or (c == '-' and curr.size() == 0))
		{
			curr.push_back(c);
		}
		else
		{
			if(curr.size() > 0)
			{	++count;
				vec.push_back(atoi(curr.c_str()));
			}
			curr = string();
			if(c != ' ' and c != '\t' and c != '\n')
			{
				//cerr << "INSTRING READIN found strange character: **" << c << "**" << endl;
			}
		}
	}
	if(curr.size() > 0)
	{
		vec.push_back(atoi(curr.c_str()));
		++count;
	}
	//cerr << "INTSTRING: READ IN " << count << " integers" << endl;
	return in;
}

	ostream& operator<<(ostream& out, const intstring& ws)
	{
		for (vector<int>::const_iterator i=ws.begin(); i!=ws.end(); i++) {
      out << *i << " ";
    }
		return out;
	}


}
