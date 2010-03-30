///////////////////////////////////////////////////////////////////////////////*
// Herman Stehouwer
// j.h.stehouwer@uvt.nl
////////////////////////////////////////////////////////////////////////////////
// Filename: suffixarray.h
////////////////////////////////////////////////////////////////////////////////
// This file is part of the suffix array package.
////////////////////////////////////////////////////////////////////////////////
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
// This file contains the implementation of the classes used in the
// suffix array package. The class suffixarray contains the
// functionality of a suffix array
// Both classes are defined in the ns_suffixarray namespace.
////////////////////////////////////////////////////////////////////////////////
// Please see the file main.cpp for a few examples on how to use the library.
// All function and variable declarations in the header are documented as well.
// Basic use is pretty straighforward:
// - Initialise the suffixarray with a sequential datatype (basic constructor).
//   It is possible to save and load to/from a file. Note that while loading
//   the original data used to build the suffixarray originally must be provided 
//   as well.
// - Ask for (indexes or counts) of (ngrams or skipgrams) possibly with wildcards.
//   Wildcars are the default value, Or whatever value you give to the function.
////////////////////////////////////////////////////////////////////////////////
// Library interface inspired from Menno Van Zaanen's
// suffixtree package (http://ilk.uvt.nl/~menno/research/software/suffixtree)
// to ensure interoperability.
// i.e. this class can be used as a drop-in replacement for those suffix trees.
//////////////////////////////////////////////////////////////////////////////*/

#ifndef __suffixarray__
#define __suffixarray__

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stack>
#include <algorithm>
#include <string>
#include <set>

namespace ns_suffixarray {
  
  class StringNotFound {
  }; // Exception
  
  template <class V>
    class suffixarray {
    
  public:
    typedef V value_type;
    typedef typename value_type::size_type size_type;
    typedef typename value_type::value_type element_type;
    typedef typename value_type::const_iterator index;
    
    
  protected:
    // SACmp class is used (anonymously) to do the sorting of the suffix array.
    class SACmp
    {
    public:
	SACmp( const value_type& content, int mi, int ma ): cont(content), min(mi), max(ma) {};
	bool operator()(size_type x, size_type y) const;
    private:
      const value_type& cont;
      int min;
	  int max;
    };
    
    // Helper class, defines an interval on the suffixarray
    // i is the left-limit, j the right-limit
    // both are INCLUSIVE!
    // so [i..j] (and Not for instance [i..j) )
    class interval
    {
    public:
      //construct an interval
    interval(size_type ii, size_type jj): i(ii), j(jj){};
      // Calculates the size of the interval. Inclusive. i.e. [0,0].size = 1
      size_type size() const { return (j-i + 1); };
      // stores the start position of the interval
      size_type i;
      // stores the end position of the interval
      size_type j;      
    };
    
    
    // This class keeps track of all the lcp, up, down and nextl values for a suffix-array.
    // Stores and retrieves the values transparently and efficiently (memory wise).
    // put in a seperate class in order to make optimalisations easyer to implement.
    class childtabs
    {
    private:
      // Stores the lcp array for lcp values < 255
      // Exceptions stord as 255 with overflow in the lcpmap map.
		std::vector<unsigned char> lcp;
      // Stores all the exceptions to this
		std::map<size_type,size_type> lcpmap;
      
      // Stores the up, down and nexl_index arrays in one place.
      // As explained under algorithm 6.5 in "Replacing suffix threes with enhances suffix arrays"
      // Stores relative indexes -128 < index < 127;
      // if the index is outside of that stored as 127 with overflow in the allmap map.
		//std::vector<signed char> all;
      // Stores the exceptions to the all vector
		//std::map<size_type,size_type> allmap;
      
		std::vector<signed char> up;
		std::map<size_type,size_type> upmap;

		std::vector<signed char> down;
		std::map<size_type,size_type> downmap;

		std::vector<signed char> nextl;
		std::map<size_type,size_type> nextlmap;

		
      // Stores a value in the all vector and allmap map.
      void
	store(std::vector<signed char>& vec, std::map<size_type,size_type>& map,size_type pos, size_type value);
      
      // Retrieves a value from the all vector and allmap map.
      // Retrieves the same value as stored by store(pos, val).
      size_type
		retrieve(std::vector<signed char>& vec, std::map<size_type,size_type>& map, size_type pos);
      
      // Returns true if the position contains a nextl value;
      bool
	isnextl(size_type pos) const;
      
      // Returns true if the position has an up value defined (stored on pos-1)
      bool
	isup(size_type pos) const;
      
      // Returns true if for the current position the down value should be stored/retrieved
      // from pos
      bool
	isdownl(size_type pos) const;
      
      // Returns true if for the current position the down value should be retrieved from
      // pos+1
      // NOTE: exception for pos == 0; add 1 to the value returned!
      bool
	isdownu(size_type pos) const;
      
    public:
      // Initialisation function.
      // To be called FIRST. size should equal the size of the suffix array to generate the values on.
      void
	init_tabs(size_type size);
      
      // Stores and lcp value at a certain position.
      void
	setlcp(size_type pos, size_type value);
      
      // Retrieves an lcp value for a certain position.
      size_type
	getlcp(size_type pos);
      
      // Stores a nextl value.
      void
	setnextl(size_type pos, size_type value);
      
      // Retrieves the correct nextl value if it should be stored on position.
      // otherwise returns (size_type) -1
      size_type
	getnextl(size_type pos);
      
      // Stores an up value if it should be stored in the correct position.
      void
	setup(size_type pos, size_type value);
      
      // Retrieves an up value that corresponds to the current position from the correct place in the storage.
      // otherwise returns (size_type) -1
      size_type
	getup(size_type pos);
      
      // Stores an down value if it should be stored in the correct position
      void
	setdown(size_type pos, size_type value);
      
      // Retrieves a down value from the current position if isdown(pos)
      // if isdownu(pos) retrieves it from the correct up position
      // otherwise returns (value_type) -1
      size_type
	getdown(size_type pos);
    };
	// Class to make a smart-building trie structure.
	// Used to quickly sort a (small) part of the data
	// Each isntance is a full Node, all nodes use the same class.
	// A Node with an empty list of children is a leaf.
	class sorttrie
	{
		private:
		// A pointer to the actuall content of the parent suffixarray
		const value_type& cont;
		// The depth of the current node compared to its index.
		size_type depth;
		// The index of the current node one the content.
		// such that it represents (a part of) the path of cont[idx].
		size_type idx;
		// A list of the children elements of this sorttrie.
		std::map<element_type,sorttrie> children;
		public:
		// Constructor, makes a new sorttrie at depth d with index i.
		// Index i denotes the important value of the sorttrie
		// It points to content[i];
		sorttrie(const value_type& content, size_type d, size_type i);
		// Is this sorttrie a leaf? (all nodes start as leaves.
		bool isLeaf() const {return children.size() == 0;};
		// Makes a new child from self (at depth+1) and adds it to the children list.
		// After this the sorttrie is no longer a leaf.
		void makeChildSelf();
		// Adds a child to the children list.
		// IF this node is a leaf call makeChildSelf first!
		void addChild(sorttrie child);
		// Returns the element in the content to which this node points.
		// so cont[idx+depth]
		element_type getelem() const;
		// Checks to see if the current node has a child which points to "el".
		bool hasChild(element_type el) const;
		// Returns a pointer to the child node which points to "el".
		// The caller should check with hasChild(el) before calling this function.
		sorttrie * getChild(element_type el);
		// Walks and adds new content to the sorttrie.
		// For simplicity we assume indexes on the same content pointer.
		void walkNadd(size_type d, size_type i);
		// Gives the begin iterator to the children map.
		typename std::map<element_type,sorttrie>::iterator begin();
		typename std::map<element_type,sorttrie>::reverse_iterator rbegin();
		// Gives the end iterator to the children map.
		typename std::map<element_type,sorttrie>::iterator end();
		typename std::map<element_type,sorttrie>::reverse_iterator rend();
		// Returns the index of the Node
		size_type getidx() const;
		// Returns a sorted vector of indexes.
		// build up using a depth-first search of the trie structure
		// with getidx() on all leaves added to the return vector.
		std::vector<size_type> getSort(sorttrie& s) const;
		std::vector<typename V::size_type> recGS(sorttrie& s) const;
		// pretty print.
		void print(std::ostream &os);
	};
  private:
    // Fills the lcp vector
    void 
      fill_lcp();
    
    // Calculates the up, down and nextl indexes in one pass.
    void
      fill_up_down_nextl();
    
    // returns the LCP value of an lcp interval.
    // this is the lcp shared by all members of the interval.
    size_type getlcp( const interval& inter);
    
    // returns a list of all the child-intervals of the given valid LCP-interval
		std::vector<interval>
      getChildIntervals( const interval& parent);
    
	// returns the larges interval.
		interval
		getRootInterval(){return interval(0,idx.size()-1);}
		
    // processes a line to add to the suffix array.
    // the line should be written out earlyer by the savearray function
    void
		processline( const std::string& line);
    
	// Attemps somewhat smarter sorting
	// fase 1 of deep-shallow sort: creates buckets.
	void
	ds();
	
	//fase 2 of deep-shallow sorting: sorts one bucket
	void
		ds2( typename std::vector<size_type>::iterator backit,  typename std::vector<size_type>::iterator frontit,int startdepth);

    // Points to the sequence on wich the suffix array is build
    const value_type& content;
    
    // holds the lcp, up, down and nextl values
    childtabs tabs;
    
    // Points to the index array, The regular suffix array.
		std::vector<size_type> idx;
    
public:
  // suffixarray creates a suffixarray based on s.
  suffixarray( const value_type& s);

  // suffixarray reads in a suffixarray from a file.
  // the value_type should be equal to the datastructure the suffixarray was created on the first time.
		suffixarray( const value_type& s, const std::string& f);

  // Stores the suffixarray in file f, returns false on failure.
  bool
    savesarray( const std::string& f);

  // find_position returns a position if w is present in the
  // suffixarray and otherwise it raises a StringNotFound exception. w need
  // not be a suffix.  wildcard indicates which element_type should be
  // considered the wildcard element.  Wildcards match any one
  // element_type.
  size_type
  find_position( const value_type& w, element_type wildcard = element_type()) const;

  // find_all_positions finds all positions of substring w occurring
  // in the suffix tree. The vector need not be ordered.  wildcard
  // indicates which element_type should be considered the wildcard
  // element.  Wildcards match any one element_type.
		std::vector<size_type>
  find_all_positions( const value_type& w, element_type wildcard = element_type()) const;

  // find_all_positions_count finds the number of positions of
  // substring w occurring in the suffix tree.  wildcard indicates
  // which element_type should be considered the wildcard element.
  // Wildcards match any one element_type.
  size_type
  find_all_positions_count( const value_type& w, element_type wildcard = element_type());

  // find_all_positions_skip finds all positions of the skipgram and returns those list of positions.
  // each position is an index on the content of the suffixarray. points at the first element of the skipgram.
  // constraints are the minimum and maximum skip between consequitive items, NOT the maximum and minimum total skip.
  // takes into account the sizes of the parts of the skipgram.
  // main argument is a vector IN ORDER of the different parts of the skipgram.
  // within each part of the skipgram it is explicitly allowed to use wildcards.
	std::vector< size_type >
		find_all_positions_skip( const std::vector<value_type>& w, size_type minskip, size_type maxskip, element_type wildcard = element_type()) const;

  // find_all_positions_skip_count returns the number of skipgrams found.
  // constraints are the minimum and maximum skip between consequitive items, NOT the maximum and minimum total skip.
  // takes into account the sizes of the parts of the skipgram.
  // main argument is a vector IN ORDER of the different parts of the skipgram.
  // within each part of the skipgram it is explicitly allowed to use wildcards.
  size_type
		find_all_positions_skip_count( const std::vector<value_type>& w, size_type minskip, size_type maxskip, element_type wildcard = element_type()) const;
  

		// print the lcp-interval tree
		// debug purposes
		void printlcpintervaltree();
		
  // print writes the suffixarray on os.
  void
		print(std::ostream &os);

			// gets an element for a suffix array positions.
			element_type getElem(int index, int depth)
			{
				element_type uit = element_type();
				if(index >= 0 && depth >= 0 && index < (int) content.size() && ( idx.at(index) + depth) < content.size())
				{
					uit = content.at(idx.at( index ) + depth);
				}
				else
				{
					std::cerr << "Asked for Erroneous element: index: " << index << " depth: " << depth << std::endl;
				}
				return uit;
			}

};

	//
	// suffixarray::SACmp public functions:
	//
	
	template <class V>
	bool 
	suffixarray<V>::SACmp::operator()(size_type x, size_type y) const
	{   
		if(x == y)
		{
			return false;
		}
		if(x + min > cont.size()){return false;}
		if(y + min > cont.size()){return false;}
		typename V::const_iterator itx = cont.begin() + min + x;
		typename V::const_iterator ity = cont.begin() + min + y;
		int m = max;
		while(--m != 0)
		{
			if(*itx == *ity){++itx;++ity;}
			else { return *itx < *ity; }
		}
		return *itx < *ity;
	}
	
	
	//
	// suffixarray::childtabs private functions:
	//
	
	template <class V>
	void
	suffixarray<V>::childtabs::store(std::vector<signed char>& vec, std::map<size_type,size_type>& map,size_type pos, size_type value)
	{
		// signed char is -128 to 127 exception positions are -128 (NO assignment, (value_type) -1) and 127 (lookup in map)
		int rel = value - pos;
		if(rel < (int) 127 && rel > (int) -128 )
		{
			vec.at(pos) = (signed char) rel;
			return;
		}
		vec.at(pos) = (signed char) 127;
		map.insert ( std::pair<size_type,size_type>(pos,value) );									
	}
	
	template <class V>
	typename V::size_type
	suffixarray<V>::childtabs::retrieve(std::vector<signed char>& vec, std::map<size_type,size_type>& map,size_type pos)
	{
		// signed char is -128 to 127 exception positions are -128 (NO assignment, (value_type) -1) and 127 (lookup in map)
		if(pos > vec.size() || pos < (size_type) 0 || pos == (size_type) -1)
		{
			return (size_type) -1;
		}
		signed char out = vec.at(pos);
		if((int) out == (int) -128)
		{
			return (size_type) -1;
		}
		if((int) out == (int) 127)
		{
			return map.find(pos)->second;
		}
		return (size_type) ((int) pos + (int) out);
	}
	
	template <class V>
	bool
	suffixarray<V>::childtabs::isnextl(size_type pos) const
	{
		// if lcptab[childtab[i].nextlindex] = lcptab[i]
		size_type lcpi = getlcp(pos);
		size_type nextli = retrieve(pos);
		size_type lcpii = getlcp(nextli);
		//cerr << "\nSARR.DEBUG isnextl pos: " << pos << endl;
		//cerr << "\tlcpi: " << lcpi <<" nextli: " << nextli << " lcpii: " << lcpii << endl;
		if(pos == nextli)
		{
			return false;
		}
		if(lcpi == lcpii)
		{
			return true;
		}
		return false;
	}
	
	template <class V>
	bool
	suffixarray<V>::childtabs::isup(size_type pos) const
	{
		if(pos == 0)
		{
			return false;
		}
		return getlcp(pos-1) > getlcp(pos);
	}
	
	template <class V>
	bool
	suffixarray<V>::childtabs::isdownl(size_type pos) const
	{
		// if lcptab[childtab[i].nextlindex] > lcptab[i]
		size_type nextli = retrieve(pos);
		size_type lcpi = getlcp(nextli);
		size_type lcpii = getlcp(pos);
		if(pos == nextli)
		{
			return false;
		}
		if(lcpi > lcpii)
		{
			return true;
		}
		return false;
	}
	
	template <class V>
	bool
	suffixarray<V>::childtabs::isdownu(size_type pos) const
	{
		return isnextl(pos);
	}
	
	//
	// suffixarray::childtabs public functions:
	//
	
	template <class V>
	void
	suffixarray<V>::childtabs::init_tabs(size_type size)
	{
		lcp = std::vector<unsigned char>(size);
		lcpmap = std::map<size_type,size_type>();
		
		//all = std::vector<signed char>(size,-128);
		//allmap = std::map<size_type,size_type>();
		
		up = std::vector<signed char>(size,-128);
		down = std::vector<signed char>(size,-128);
		nextl = std::vector<signed char>(size,-128);
		
		upmap = std::map<size_type,size_type>();
		downmap = std::map<size_type,size_type>();
		nextlmap = std::map<size_type,size_type>();

	}
	
	template <class V>
	void
	suffixarray<V>::childtabs::setlcp(size_type pos, size_type value)
	{
		if(pos >= lcp.size())
		{
			std::cerr << "ERROR ERROR :: POS IS TOT LARGE:: " << pos << std::endl;
			exit(0);
		}
		// unsigned char is [0,255] 255 stores exceptions
		if(value < (size_type) 255)
		{
			lcp.at(pos) = (unsigned char) value;
			return;
		}
		lcp.at(pos) = (unsigned char) 255;
		lcpmap.insert ( std::pair<size_type,size_type>(pos,value) );
	}
	
	template <class V>
	typename V::size_type
	suffixarray<V>::childtabs::getlcp(size_type pos)
	{
		// unsigned char is [0,255] 255 stores exceptions
		if(pos > lcp.size() || pos < 0 || pos == (size_type) -1)
		{
			return (size_type) -1;
		}
		unsigned char out = lcp.at(pos);
		if((size_type) out < 255)
		{
			return (size_type) out;
		}
		return lcpmap.find(pos)->second;
	}
	
	template <class V>
	void
	suffixarray<V>::childtabs::setnextl(size_type pos, size_type value)
	{
		store(nextl,nextlmap,pos, value);
	}
	
	template <class V>
	typename V::size_type
	suffixarray<V>::childtabs::getnextl(size_type pos)
	{
		//if(isnextl(pos))
		//{
			return retrieve(nextl,nextlmap,pos);
		//}
		//return (size_type) -1;
	}
	
	template <class V>
	void
	suffixarray<V>::childtabs::setup(size_type pos, size_type value)
	{
		//if(isup(pos))
		//{
		//	--pos;
			store(up,upmap,pos, value);
		//}
	}
	
	template <class V>
	typename V::size_type
	suffixarray<V>::childtabs::getup(size_type pos)
	{
		/*if(pos >= lcp.size() 
		   or pos < 1
		   or not isup(pos))
		{
			return (size_type) -1;
		}
		--pos;*/
		return retrieve(up,upmap,pos);
	}
	
	template <class V>
	void
	suffixarray<V>::childtabs::setdown(size_type pos, size_type value)
	{
		//if(isdownl(pos));
		//{
			store(down, downmap,pos,value);
		//}
	}
	
	template <class V>
	typename V::size_type
	suffixarray<V>::childtabs::getdown(size_type pos)
	{
		//if(isdownl(pos))
		//{
			return retrieve(down, downmap, pos);
		/*}
		if(isdownu(pos))
		{
			if(pos == 0)
			{
				return getup(pos+2)+1;
			}
			return getup(pos+2);
		}
		return (size_type) -1;*/
	}
	
	//
	// suffixarray::sorttrie public functions
	//
	template <class V>
	suffixarray<V>::sorttrie::sorttrie(const value_type& content, size_type d, size_type i): cont(content)
	{
		depth = d;
		idx = i;
	}
	
	template <class V>
	void
	suffixarray<V>::sorttrie::makeChildSelf()
	{
		addChild(sorttrie( cont, depth+1, idx ));
	}
	
	template <class V>
	void
	suffixarray<V>::sorttrie::addChild(sorttrie child)
	{
		children.insert(std::pair<element_type,sorttrie>(child.getelem(),child));
	}
	
	template <class V>
	typename V::value_type
	suffixarray<V>::sorttrie::getelem() const
	{
		if(depth >= 0)
		{
			return cont.at(idx+depth);
		}
		// Note: can only occur on the root of the tree
		// And only when the print function is called (normally one only looks at the children)
		// And only with a starting depth of -1 (or 0) depends on how you look at it
		// value is undefined so we return the default value for the type.
		element_type a;
		return a;
	}
	
	template <class V>
	bool
	suffixarray<V>::sorttrie::hasChild(element_type el) const
	{
		return(children.find(el) != children.end());
	}
	
	template <class V>
	typename suffixarray<V>::sorttrie *
	suffixarray<V>::sorttrie::getChild(element_type el)
	{
		sorttrie * uit = &(children.find(el)->second);
		return uit;
	}
	
	template <class V>
	void
	suffixarray<V>::sorttrie::walkNadd(size_type d, size_type i)
	{
		sorttrie *s = this;
		while(1)
		{
			if(s->isLeaf()){s->makeChildSelf();}
			element_type el = cont[i+d];
			if(s->hasChild(el))
			{
				s = s->getChild(el);
				++d;
			}
			else
			{
				s->addChild(sorttrie( cont, d, i ));
				return;
			}
		}
	}
	
	template <class V>
	typename std::map<typename V::value_type,typename suffixarray<V>::sorttrie>::iterator
	suffixarray<V>::sorttrie::begin()
	{
		return children.begin();
	}
	
	template <class V>
	typename std::map<typename V::value_type,typename suffixarray<V>::sorttrie>::reverse_iterator
	suffixarray<V>::sorttrie::rbegin()
	{
		return children.rbegin();
	}
	
	
	template <class V>
	typename std::map<typename V::value_type,typename suffixarray<V>::sorttrie>::iterator
	suffixarray<V>::sorttrie::end()
	{
		return children.end();
	}
	
	template <class V>
	typename std::map<typename V::value_type,typename suffixarray<V>::sorttrie>::reverse_iterator
	suffixarray<V>::sorttrie::rend()
	{
		return children.rend();
	}
	
	
	template <class V>
	typename V::size_type
	suffixarray<V>::sorttrie::getidx() const
	{
		return idx;
	}
	template <class V>
	std::vector<typename V::size_type> 
	suffixarray<V>::sorttrie::recGS(sorttrie& s) const
	{
		std::vector<typename V::size_type> uit;
		if(s.isLeaf())
		{
			uit.push_back(s.getidx());
		}
		else
		{
			typename std::map<element_type,sorttrie>::iterator it = s.begin();
			while(it != s.end())
			{
				std::vector<typename V::size_type>temp = recGS((*it).second);
				typename std::vector<typename V::size_type>::iterator itt = temp.begin();
				while(itt != temp.end())
				{
					uit.push_back(*itt);
					++itt;
				}
				++it;
			}
		}
		return uit;
	}
	
	template <class V>
	std::vector<typename V::size_type>
	suffixarray<V>::sorttrie::getSort(sorttrie& strie) const
	{
		std::vector<typename V::size_type> uit;
		//uit = recGS(strie);
		//return uit;
		std::stack<sorttrie*> s;
		s.push(&strie);
		while(s.size() > 0)
		{
			sorttrie * curr = s.top();
			s.pop();
			if(curr->isLeaf())
			{
				//std::cerr << "FOUND LEAF " << std::endl;
				uit.push_back(curr->getidx());
			}
			else
			{
				typename std::map<element_type,sorttrie>::reverse_iterator it = curr->rbegin();
				while(it != curr->rend())
				{
					s.push(&((*it).second));
					++it;
				}
			}
		}
		return uit;
	}
	
	template <class V>
	void
	suffixarray<V>::sorttrie::print(std::ostream &os) {
		// print the sorttrie
		// purely for debugging purposes.
		size_type d = 0;
		// It is very convenient that for pretty printing the depth is a known value.
		while(d < depth)
		{
			os << "-\t";
			++d;
		}
		os << cont[depth+idx];
		os << std::endl;
		typename std::map<element_type,sorttrie>::iterator bit = begin();
		typename std::map<element_type,sorttrie>::iterator eit = end();
		while(bit != eit)
		{
			std::pair<element_type,sorttrie> curr = *bit;
			curr.second.print(os);
			++bit;
		}
	}
	
	
	//
	// suffixarray private functions:
	//
	
	template <class V>
	void
	suffixarray<V>::fill_lcp()
	{
		// GENERATE all lcp values
		// Implemented from Algorithm 4.1 from "Replacing sufﬁx trees with enhanced sufﬁx arrays"
		// Abouelhoda, Kurtz, Ohlebusch 2003
		
		std::vector<size_type> rank(idx.size());
		for(size_type i = 0; i < idx.size();i++)
		{
			if(idx.at(i) > rank.size())
			{
				std::cerr << "ERROR ERROR, idx.at(i) too large. i: " << i << " idx.at(i):" << idx.at(i) << std::endl;
			}
			rank.at(idx.at(i)) = i;
		}
		size_type h = 0;
		for( size_type i = 0; i < idx.size();i++)
		{
			if(rank.at(i) > 0 and rank.at(i) <= idx.size())
			{
				int k = idx.at(rank.at(i)-1);
				while(content.at(i+h) == content.at(k+h))
				{
					h++;
				}
				tabs.setlcp( rank.at(i), h);
				if(h > 0)
				{
					h--;
				}
			} 
		}
	}
	
	template <class V>
	void
	suffixarray<V>::fill_up_down_nextl()
	{
		// Fills the nextlIndex array (actually vector)
		// Value -1 denotes empty
		// Implemented from Algorithms 6.2 and 6.5 from "Replacing suffix threes with enhances suffix arrays"
		size_type lastIndex = -1;
		std::stack<size_type> updownstack;
		updownstack.push(0);
		std::stack<size_type> nextlstack;
		nextlstack.push(0);
		for( size_type i = 1; i < idx.size(); i++)
		{
			while(tabs.getlcp(i) < tabs.getlcp(updownstack.top()))
			{
				lastIndex = updownstack.top();
				updownstack.pop();
				if((tabs.getlcp(i) <= tabs.getlcp(updownstack.top())) 
				   && (tabs.getlcp(updownstack.top()) != tabs.getlcp(lastIndex)))
				{
					tabs.setdown(updownstack.top(),lastIndex);
				}
			}
			if(lastIndex != (size_type) -1)
			{
				tabs.setup(i,lastIndex);
				lastIndex = -1;
			}
			updownstack.push(i);
			while(tabs.getlcp(i) < tabs.getlcp(nextlstack.top()))
			{
				nextlstack.pop();
			}
			if(tabs.getlcp(i) == tabs.getlcp(nextlstack.top()))
			{
				tabs.setnextl(nextlstack.top(), i);
				nextlstack.pop();
			}
			nextlstack.push(i);
			
		}
	}
	
	template <class V>
	typename V::size_type
	suffixarray<V>::getlcp( const interval& inter)
	{
	//	std::cerr << "getLCP:: [" << inter.i << "," << inter.j <<"] size: " << inter.size() << std::endl;
		if(inter.size() == 1)
		{
	//		std::cerr <<" inter.size() == 1" << std::endl;
			// size of the interval is 1, lcp is clearly the size of the suffix-array there!
			return content.size() - idx.at(inter.i) -1;
		}
		if(inter.j+1 >= idx.size())
		{
			return 0;
		}		
		if((inter.j+1) < idx.size() 
		   && inter.i < tabs.getup(inter.j+1) 
		   && tabs.getup(inter.j+1) <= inter.j)
		{
			return tabs.getlcp(tabs.getup(inter.j+1));
		}
		else
		{
			return tabs.getlcp(tabs.getdown(inter.i));
		}
	}
	
	
	template <class V>
	std::vector<typename suffixarray<V>::interval>
	suffixarray<V>::getChildIntervals( const interval& parent)
	{
		// Implemented from Algoarithms 6.7 from "Replacing suffix threes with enhances suffix arrays"
		std::vector<interval> found;
		size_type i1;
		size_type i2;
		if(parent.i == parent.j || parent.i >= idx.size() || parent.j >= idx.size())
		{ // Interval of size 1, doesn't get smaller than this.
			return found;
		}
		//std::cerr << "SARR.gci: finding children for interval: [" << parent.i << "," << parent.j << "]" << std::endl;
		if( parent.j+1 < idx.size())
		{
			if(parent.i < tabs.getup(parent.j+1) && tabs.getup(parent.j+1) != (size_type) -1 )
			{
				i1 = tabs.getup(parent.j+1);
			}
			else
			{
				if(tabs.getnextl(parent.i) == (size_type) -1)
				{
					i1 = tabs.getdown(parent.i);
				}
				else
				{
					i1 = tabs.getnextl(parent.i);
				}
			}
		}
		else
		{
			i1 = tabs.getnextl(parent.i);
		}
		found.push_back(interval(parent.i,i1-1));
		//
		// NOTE: make sure comparison with (size_type) -1 is valid!
		//
		//std::cerr << "test " << std::endl;
		while(tabs.getnextl(i1) != (size_type) -1 and i1 < parent.j)
		{
			i2 = tabs.getnextl(i1);
			//std::cerr << "SARR.gci: found child: [" << i1 << "," << i2-1 << "]" << std::endl;
			found.push_back(interval(i1,i2-1));
			i1 = i2;
		}
		//std::cerr << "test2 " << std::endl;
		found.push_back(interval(i1,parent.j));
		return found;
	}
	
	template <class V>
	void
	suffixarray<V>::processline( const std::string& line)
	{
		std::stringstream ss(line);
		std::string item;
		std::vector<std::string> elems;
		while(getline(ss, item, '\t')) {
			elems.push_back(item);
		}
		std::vector<std::string>::const_iterator it = elems.begin();
		// index
		size_type i = (size_type) atoi((*it).c_str());
		size_type curr;
		// idx.at
		++it;
		if(*it != "-")
		{
			curr = (size_type) atoi((*it).c_str());
			idx.at(i) = curr;
		}
		// lcp
		++it;
		if(*it != "-")
		{
			curr = (size_type) atoi((*it).c_str());
			tabs.setlcp(i,curr);
		}
		// up
		++it;
		if(*it != "-")
		{
			curr = (size_type) atoi((*it).c_str());
			tabs.setup(i,curr);
		}
		// down
		++it;
		if(*it != "-")
		{
			curr = (size_type) atoi((*it).c_str());
			tabs.setdown(i,curr);
		}
		// nextl
		++it;
		if(*it != "-")
		{
			curr = (size_type) atoi((*it).c_str());
			tabs.setnextl(i,curr);
		}
		
	}
	
	template <class V>
	void
	suffixarray<V>::ds()
	{
		// Attempts smart-ish sorting by first creating buckets (with depth 1)
		// and then sorting each of these buckets seperately.
		std::cerr << "\t\tSARR.DS starting" << std::endl;
		sort(idx.begin(),idx.end(), SACmp(content,0,2));
		typename std::vector<size_type>::iterator searchit = idx.begin();
		typename std::vector<size_type>::iterator backit = idx.begin();
		element_type curra = content[*searchit];
		element_type currb = content[(*searchit)+1];
		++searchit;
		while(searchit != idx.end())
		{
			if
				(
				 *searchit + 1 < content.size() &&
				 (curra != content[*searchit] || currb != content[(*searchit)+1])
				 )
			{
				curra = content[*searchit];
				currb = content[(*searchit)+1];
				if(distance(backit,searchit) > 1)
				{
					ds2(backit, searchit,2);
				}
				backit = searchit;
			}
			
			++searchit;
		}
		// NOTE: last two elements are already really sorted!
		// so we don't sort those.
		--searchit;
		if(backit != searchit)
		{
			--searchit;
			// If the last bucket actually contains something sort.
			if(backit != searchit)
			{
				if(distance(backit,searchit) > 1)
				{
					ds2(backit, searchit,2);
				}
			}
		}
		std::cerr << "\t\tSARR.DS done!" << std::endl;
	}
	
	template <class V>
	void
	suffixarray<V>::ds2(typename std::vector<size_type>::iterator backit, typename std::vector<size_type>::iterator frontit, int startdepth)
	{
		if(distance(backit,frontit) < 5)
		{ // A bit of a magic number, but no real need to build a tree for really small buckets!
			sort(backit,frontit,SACmp(content,startdepth,-1));
			return;
		}
		
		typename std::vector<size_type>::iterator it = backit;
		// build the trie
		sorttrie root(content, startdepth-1, *it);
		root.makeChildSelf();
		++it;
		int count = 1;
		while(it != frontit)
		{
			//root.print(std::cerr);
			root.walkNadd(startdepth, *it);
			++it;
			++ count;
		}
		//std::cerr << "Inserted : " << count << " elements in the tree" << std::endl;
		// depth-first traversal of the trie
		std::vector<size_type> sorted = root.getSort(root);
		// change values to sorted values
		it = backit;
		if(sorted.size() != (typename std::vector<size_type>::size_type) distance(backit, frontit))
		{
			std::cerr << "SIZE MISMATCH! :: " << sorted.size() << " sorted elements for " << distance(backit,frontit) << "positions!" << std::endl;
			root.print(std::cerr);
			exit(0);
		}
		typename std::vector<size_type>::iterator itt = sorted.begin();
		while(it != frontit)
		{
			size_type temp = (*itt);
			(*it) = temp;
			++it;
			++itt;
		}
	}
	
	//
	// Suffixarray Public functions:
	//
		
	template <class V>
	suffixarray<V>::suffixarray(const value_type& s): content(s)
	{
		// NOTE: IMPORTANT:
		// LAST ELEMENT OF s _MUST_ BE UNIQUE
		// LAST ELEMENT OF s _MUST_ BE > THAN ALL OTHER ELEMENTS
		// Constructor initialising an array.
		// Initialise content and size
		std::cerr << "SARR CONSTRUCTOR" << std::endl;
		size_type i = 0;
		std::cerr << "\tfilling index" << std::endl;
		while(i < s.size())
		{
			idx.push_back(i);
			++i;
		}
		std::cerr << "\tsorting index" << std::endl;
		// sort (make the suffix array a suffix array)
		//sort(idx.begin(),idx.end(), SACmp( content,0,-1 ));
		ds();
		
		// generate the lcp table in the vector<int> lcp
		tabs = childtabs();
		tabs.init_tabs( idx.size() );
		
		std::cerr << "\tgenerating lcp" << std::endl;
		fill_lcp();
		// up, down, nexlindex
		std::cerr << "\tgenerating up, down and nextl index" << std::endl;
		fill_up_down_nextl();
		std::cerr << "SARR CONSTRUCTOR DONE" << std::endl;
	}
	
	template <class V>
	suffixarray<V>::suffixarray( const value_type& s, const std::string& f ): content( s )
	{
		// Initialise a suffixarray which has been previously stored to disk!
		tabs = childtabs();
		tabs.init_tabs( s.size());
		
		std::ifstream myfile (f);
		std::string line;
		if (myfile.is_open())
		{
			while (! myfile.eof() )
			{
				getline (myfile,line);
				processline(line);
			}
			myfile.close();
		}
	}
	
	template <class V>
	bool
	suffixarray<V>::suffixarray::savesarray( const std::string& f)
	{
		// save array to file
		std::ofstream myfile (f);
		if (myfile.is_open())
		{
			print(myfile);
			myfile.close();
			return true;
		}
		std::cerr << "Unable to open file" << std::endl;
		return false;
	}
	
	
	template <class V>
	typename suffixarray<V>::size_type
	suffixarray<V>::find_position( const value_type& w, element_type wildcard) const {
		// Check if sentence is a substring in the suffix array. Throw a
		// StringNotFound if it is not.
		size_type depth = 0;
		std::vector<interval> tosearch;
		tosearch.push_back(interval(0,idx.size()-1));
		while(depth < w.size() && tosearch.size() > 0)
		{
			std::vector<interval> tosearch2;
			for( size_type i = 0; i < tosearch.size(); i++)
			{
				interval curr = tosearch.at(i);
				std::vector<interval> nieuw;
				if(depth < getlcp(curr) || curr.size() == 1)
				{ // no need to generate children! interval is equal for the first getlcp() elements
					// We also don't need to generate children if the interval has size 1
					nieuw.push_back(curr);
				}
				else
				{ // we need to generate children!
					nieuw = getChildIntervals(tosearch.at(i));
				}
				for( size_type in = 0; in < nieuw.size(); in++)
				{
					// Filter the children and if they are still OK at this depth add them for the next round!
					element_type tocomp = w.at(depth);
					if(tocomp == wildcard)
					{
						tosearch2.push_back(nieuw.at(in));
					}
					else
					{
						curr = nieuw.at(in);
						if( ( idx.at(curr.i) + depth) < content.size())
						{
							element_type tocomp2 = content.at(idx.at( curr.i ) + depth);
							if(tocomp == tocomp2)
							{
								tosearch2.push_back(nieuw.at(in));
							}
						}
					}
				}
			}
			tosearch = tosearch2;
			depth++;
		}
		if(tosearch.size() > 0)
		{
			return idx.at(tosearch.at(0).i);
		}
		throw StringNotFound();
	}
	
	template <class V>
	std::vector<typename suffixarray<V>::size_type>
	suffixarray<V>::find_all_positions( const value_type& w, element_type wildcard) const {
		std::vector<size_type> results;
		size_type depth = 0;
		std::vector<interval> tosearch;
		tosearch.push_back(interval(0,idx.size()-1));
		while(depth < w.size() && tosearch.size() > 0)
		{
			std::vector<interval> tosearch2;
			for( size_type i = 0; i < tosearch.size(); i++)
			{
				interval curr = tosearch.at(i);
				std::vector<interval> nieuw;
				if(depth < getlcp(curr) || curr.size() == 1)
				{ // no need to generate children! interval is equal for the first getlcp() elements
					// We also don't need to generate children if the interval has size 1
					nieuw.push_back(curr);
				}
				else
				{ // we need to generate children!
					nieuw = getChildIntervals(tosearch.at(i));
				}
				for( size_type in = 0; in < nieuw.size(); in++)
				{
					// Filter the children and if they are still OK at this depth add them for the next round!
					element_type tocomp = w.at(depth);
					if(tocomp == wildcard)
					{
						tosearch2.push_back(nieuw.at(in));
					}
					else
					{
						curr = nieuw.at(in);
						if( ( idx.at(curr.i) + depth) < content.size())
						{
							element_type tocomp2 = content.at(idx.at( curr.i ) + depth);
							if(tocomp == tocomp2)
							{
								tosearch2.push_back(nieuw.at(in));
							}
						}
					}
				}
			}
			tosearch = tosearch2;
			depth++;
		}
		for( size_type i = 0; i < tosearch.size(); i++)
		{
			interval curr = tosearch.at(i);
			for(size_type j = curr.i; j <= curr.j;j++)
			{
				results.push_back(idx.at(j));
			}
		}
		return results;
	}
	
	
	template <class V>
	typename suffixarray<V>::size_type
	suffixarray<V>::find_all_positions_count( const value_type& w, element_type wildcard) {
		size_type result=0;
		size_type depth = 0;
		std::vector<interval> tosearch;
		tosearch.push_back(getRootInterval());
		//std::cerr << "STARTING FAPC, query: " << w << std::endl;
		while(depth < w.size() && tosearch.size() > 0)
		{
			//std::cerr << " FAPC at depth: " << depth << " number of items to search: " << tosearch.size() << std::endl;
			std::vector<interval> tosearch2;
			for( size_type i = 0; i < tosearch.size(); i++)
			{
				interval curr = tosearch.at(i);
				std::vector<interval> nieuw;
				if(depth < getlcp(curr) || curr.size() == 1)
				{ // no need to generate children! interval is equal for the first getlcp() elements
					// We also don't need to generate children if the interval has size 1
					nieuw.push_back(curr);
				}
				else
				{ // we need to generate children!
					nieuw = getChildIntervals(tosearch.at(i));
				}
				for( size_type in = 0; in < nieuw.size(); in++)
				{
					// Filter the children and if they are still OK at this depth add them for the next round!
					element_type tocomp = w.at(depth);
					if(tocomp == wildcard)
					{
						tosearch2.push_back(nieuw.at(in));
					}
					else
					{
						curr = nieuw.at(in);
						if( ( idx.at(curr.i) + depth) < content.size())
						{
							element_type tocomp2 = content.at(idx.at( curr.i ) + depth);
							if(tocomp == tocomp2)
							{
								tosearch2.push_back(nieuw.at(in));
							}
						}
					}
				}
			}
			tosearch = tosearch2;
			depth++;
		}
		for( size_type i = 0; i < tosearch.size(); i++)
		{
			interval curr = tosearch.at(i);
			result += curr.size();
		}
		return result;
	}
	
	template <class V>
	typename std::vector< typename suffixarray<V>::size_type >
	suffixarray<V>::find_all_positions_skip( const std::vector< value_type >& w, size_type minskip, size_type maxskip, element_type wildcard) const
	{
		std::vector< size_type > uit;
		std::vector< std::vector< size_type > > idx;
		// get indexes for all subgrams.
		{
			typename std::vector< value_type >::const_iterator it = w.begin();
			while(it != w.end())
			{
				std::vector<size_type> subgram = find_all_positions(*it,wildcard);
				// they need to be sorted later on in the algorithm, might as well do it here :)
				sort(subgram.begin(),subgram.end());
			
				idx.push_back(subgram);
				++it;
			}
		}
		
		if(idx.size() != w.size())
		{ // Only 0 or 1 subgram, USELESS to use this method!
			return uit;
		}
		if(idx.size() == 1)
		{
			// USELESS but return the correct thing anyway
			return idx.at(0);
		}
		if(idx.size() == 0)
		{
			return uit;
		}
		
		// vector of iterators (pointers) to efficiently walk all the sets of indexes.
		typename std::vector< std::pair< typename std::vector<size_type>::iterator, typename std::vector<size_type>::iterator > > iters;
		// init these pointers:
		{
			typename std::vector< typename std::vector< size_type > >::iterator it = idx.begin();
			while(it != idx.end())
			{
				iters.push_back(std::pair< typename std::vector<size_type>::iterator, typename std::vector<size_type>::iterator >((*it).begin(),(*it).end()));
				++it;
			}
		}		
		// Do magic ...
		// basically a waterfall model.
		// while there is still items in the first one, compare tot he second, compare to the third, etc.
		std::pair< typename std::vector<size_type>::iterator, typename std::vector<size_type>::iterator > & first = iters.at(0);
		typename std::vector<  std::pair< typename std::vector<size_type>::iterator, typename std::vector<size_type>::iterator > >::iterator prev = iters.begin();
		typename std::vector<  std::pair< typename std::vector<size_type>::iterator, typename std::vector<size_type>::iterator > >::iterator next = iters.begin()+1;
		while(first.first != first.second)
		{
			// If the index in the next position is smallel increase the next->first iterator.
			while(*(next->first) <= *(prev->first))
			{
				++(next->first);
				if(next->first == next->second)
				{ // we are at the end of one of the iterators: we are done.
					return uit;
				}
			}
			if(
				*(prev->first) + maxskip >= *(next->first) // If the skip is not too large.
				&& // AND
				*(prev->first) + minskip < *(next->first) // If the skip is not too small.
			   )
			{ // prev and next point to something good! check for the next set!
				++prev;
				++next;
				if(next == iters.end())
				{ // We found one!
					uit.push_back(*(first.first));
					// we added it, so we don't need to point to it anymore.
					++(first.first);
					// reset
					prev = iters.begin();
					next = iters.begin()+1;
				}
			}
			else
			{
				if(*(prev->first) + minskip >= *(next->first))
				{ // Skip is too small: Up the next pointer.
					++(next->first);
					if(next->first == next->second)
					{ // we are at the end of one of the iterators: we are done.
						return uit;
					}
				}
				else
				{ // Skip is too large apparently! This means we have to up the prev pointer.
					++(prev->first);
					if(prev->first == prev->second)
					{ // we are at the end of one of the iterators: we are done.
						return uit;
					}
					// This does mean that our previous set does not compute!
					if(prev != iters.begin())
					{
						// If we are not at the beginning, we go one step back.
						--prev;
						--next;
					}
				}
			}
		}
		return uit;
	}
	
	template <class V>
	typename V::size_type
	suffixarray<V>::find_all_positions_skip_count( const std::vector< value_type >& w, size_type minskip, size_type maxskip, element_type wildcard) const
	{
		return (size_type) find_all_positions_skip(w,minskip,maxskip,wildcard).size();
	}
	
	template <class V>
	void 
	suffixarray<V>::printlcpintervaltree()
	{
		std::stack< std::pair<int,interval> > s;
		s.push(std::pair<int,interval>(0,getRootInterval()));
		while(! s.empty())
		{
			std::pair<int,interval> curr = s.top();
			s.pop();
			int d = curr.first;
			interval cint = curr.second;
			while(d > 0)
			{
				std::cerr << "\t";
				d--;
			}
			d = curr.first;
			d++;
			std::cerr << getlcp(cint) << "-[" << cint.i << "," << cint.j << "] == ";
			if(getlcp(cint)+cint.i >= idx.size())
			{
				std::cerr << getElem(cint.i, getlcp(cint)-1) << std::endl;
			}
			else
			{
				std::cerr << getElem(cint.i, getlcp(cint) ) << std::endl;
			}
			std::vector<interval> children = getChildIntervals(cint);
			typename std::vector<interval>::reverse_iterator it = children.rbegin();
			while(it != children.rend())
			{
				s.push(std::pair<int,interval>(d,*it));
				++it;
			}
		}
	}

	
	template <class V>
	void
	suffixarray<V>::print(std::ostream &os) {
		// print the suffixtree on os.
		// NOTE: Don't change unless you also change file reading! This function is used to print the suffixarray to a file.
		for(int i = 0; i < (int) idx.size();i++)
		{
			os << i << "\t";
			os << idx.at(i) << "\t";
			if(tabs.getlcp((size_type)i) == (size_type) -1)
			{
				os << "-\t";
			}
			else
			{
				os << tabs.getlcp((size_type)i) << "\t";
			}
			if(tabs.getup((size_type)i) == (size_type) -1)
			{
				os << "-\t";
			}
			else
			{
				os << tabs.getup((size_type)i) << "\t";
			}
			if(tabs.getdown((size_type)i) == (size_type) -1)
			{
				os << "-\t";
			}
			else
			{
				os << tabs.getdown((size_type)i) << "\t";
			}
			if(tabs.getnextl((size_type)i) == (size_type) -1)
			{
				os << "-\t";
			}
			else
			{
				os << tabs.getnextl((size_type)i) << "\t";
			}
			os << std::endl;
		}
	}	
	
	// operator<< writes t on os using the print function of the
	// suffixtree class.
	template <class V>
	std::ostream &
	operator<<(std::ostream &os,
			   const suffixarray<V> &t) {
		t.print(os);
		return os;
	}
	
}
#endif // __suffixtree__
// end of file: suffixtree.h
