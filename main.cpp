///////////////////////////////////////////////////////////////////////////////*
// Herman Stehouwer
// j.h.stehouwer@uvt.nl
////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
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
////////////////////////////////////////////////////////////////////////////////
// This file is part of the suffix array package.
////////////////////////////////////////////////////////////////////////////////
// This file contains two simple examples of using the suffixarray package.
// One with a simple string (not commented)
// And one with a list of integers read in from a file.
//////////////////////////////////////////////////////////////////////////////*/

#include <sstream>
#include <iostream>
#include <string>
#include "intstring.h"
#include "wordstring.h"
#include <vector>
#include "suffixarray.h"
#include <fstream>
#include <cstdlib>
#include <limits>

#define HAVE_GETOPT_H

#ifdef HAVE_GETOPT_H
#define GNU_SOURCE
#include <getopt.h>
#else
extern "C" {
  char* optarg;
  extern int optind, opterr, optopt;
  struct option { const char *name; int has_arg; int *flag; int val; };
#define no_argument            0
#define required_argument      1
#define optional_argument      2
#ifdef HAVE_GETOPT_LONG_ONLY
  extern int getopt_long_only (int argc, char * const argv[],
    const char *optstring, const struct option *longopts, int
    *longindex);
#else
#warning \
Gnu Getopt Library not found: \
cannot implement long option handling

  extern int getopt(int argc, char* const argv[], const char*
      optstring);
  inline int getopt_long_only(int argc, char * const argv[], const char
      *optstring, const struct option *longopts, int *longindex) {
    return getopt(argc, argv, optstring);
  }
#endif
} // extern "C"
#endif // #ifdef HAVE_GETOPT_H #else

static struct option long_options[] = {
  {"help", no_argument, 0, 'h'},
  {"file", required_argument, 0, 'f'},
  {0, 0, 0, 0}
};

using namespace std;
using namespace ns_suffixarray;

typedef suffixarray<wordstring> sa_string;
typedef suffixarray<intstring> sa_int;
typedef suffixarray<string> sasa;

string program_name;

void usage() {
  cerr << "Usage: " << program_name << "[OPTION]..." << endl;
  cerr << "This program reads in a corpus and stores it in a ";
  cerr << "suffixarray.  It then returns the number of occurrences ";
  cerr << "of strings in the corpus." << endl;
  cerr << "  -h, --help        ";
  cerr << "Show this help and exit" << endl;
  cerr << "  -f, --file FILE   ";
  cerr << "Filename of the corpus to be read" << endl;
  cerr << "  -s, --server PORT ";
  cerr << "Turn server mode on, listening on the port" << endl;
  exit(0);
}

int
main(int argc, char* argv[]) {
  program_name = argv[0];

  ifstream is;

  // Handle arguments
  int opt;
  int option_index;
  const char* optstring="hf:s:";
  while ((opt = getopt_long_only(argc, argv, optstring, long_options,
          &option_index)) !=-1){
    switch (opt) {
      case 'h':
        usage();
        break;
      case 'f':
        is.open (optarg, ios::in );
        if (!is.good()) {
          cerr << "cannot open input file " << optarg << endl;
          exit (-1);
        }
        break;
      default:
        cerr << "unknown argument " << opt << endl;
        exit (-1);
    }
  }
  if (!is.is_open()) {
    cerr << "input file is not open" << endl;
    exit (-1);
  }


	wordstring word;
	is >> word;
	//word.push_back(numeric_limits<int>::max());
	word.push_back("~~~~~~~~~~~~~");
	sa_string tree(word);
	is.close();
	
  sa_string::size_type result;
    cerr << "STREE: SUFFIXARRAY Ready!" << endl;
    char input[1000000];
    while ( cin.getline(input, 1000000) ) {  // read the string and write it back
      //cerr << "STree: RECIEVED ***" << input << "***"<< endl;
	wordstring totest;
      istringstream a (string(input), istringstream::in);
      a >> totest;
      result = tree.find_all_positions_count(totest);
      //cerr << "SARRAY: Sending result: ***"<<result <<"***"<< endl;
      cout << result << endl;
    }
}
// end of file: main.cpp
