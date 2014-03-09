#ifndef ABSTRACT_SIMPLEX_H
#define ABSTRACT_SIMPLEX_H

#include <vector> //std::vector
#include <initializer_list>  // std::initializer_list
#include <iostream> //cout (debug only)
#include <algorithm> //sort, unique
#include "simplex_boundary.h"
namespace ctl {

//T must be a type with `<` a strict-weak-ordering
template< typename T>
class Abstract_simplex {
	private:
	typedef typename std::vector< T> Vector;
	typedef Abstract_simplex< T> Self;
	typedef std::initializer_list< T> Init_list;
	public:
	typedef std::size_t size_t;
	typedef typename Vector::value_type value_type;
	typedef typename Vector::value_type vertex_type;
	typedef typename Vector::const_iterator const_iterator;
	typedef typename Vector::const_reverse_iterator const_reverse_iterator;
	typedef typename Vector::iterator iterator;
	typedef typename Vector::reverse_iterator reverse_iterator;
	public:
	Abstract_simplex(): vertices(){};
	Abstract_simplex( std::size_t d) { vertices.reserve( d); }
	Abstract_simplex( const Init_list & il) : vertices( il) {
		sort( vertices.begin(), vertices.end() );
		vertices.erase( unique( vertices.begin(), vertices.end() ), 
				vertices.end() );

	}
	template< typename Iterator>
	Abstract_simplex( const Iterator begin, 
			  const Iterator end): vertices( begin,end){};
	Abstract_simplex( const Self & from): vertices( from.vertices){};

	iterator       begin()	        { return vertices.begin(); }
	const_iterator begin()  const	{ return vertices.begin(); }
	
	iterator         end()	  	{ return vertices.end();   }
	const_iterator   end()  const	{ return vertices.end();   }
	
	reverse_iterator        rbegin()	{ return vertices.rbegin(); }
	const_reverse_iterator  rbegin()  const	{ return vertices.rbegin(); }
	
	reverse_iterator         rend()	        { return vertices.rend(); }
	const_reverse_iterator   rend()  const	{ return vertices.rend(); }

	std::size_t       size() const	{ return vertices.size(); 	}
	std::size_t  dimension() const	{ return size()-1; 	  	}

	iterator insert( const vertex_type & v){
	      iterator pos = std::lower_bound( begin(), end(), v);
	      if(pos != end() && *pos == v) { return pos; }
	      return vertices.insert( pos, v);
	}

	//TODO: sort, merge, erase might be more efficient..
	//Assumes il is sorted
	void insert( const Init_list & il){
	        std::size_t offset = vertices.size();
		vertices.insert( vertices.end(), il.begin(), il.end());
	        std::inplace_merge( vertices.begin(), 
				    vertices.begin()+offset, vertices.end());
		vertices.erase( unique( vertices.begin(), vertices.end() ), 
				vertices.end() );
	
	}
	template< typename Iterator>
	void insert( Iterator first, Iterator last){
	        std::size_t offset = vertices.size();
		vertices.insert( vertices.end(), first, last);
		std::inplace_merge( vertices.begin(), 
				    vertices.begin()+offset, vertices.end());
		vertices.erase( unique( vertices.begin(), vertices.end() ), 
				vertices.end() );
	}
	iterator remove( const vertex_type v){
		iterator pos = std::lower_bound( begin(), end(), v);
		//element not in list
		if (pos == end()){ return pos; }
		return vertices.erase( pos);
	}
	iterator remove( iterator first, iterator last){
		return vertices.erase( first, last);
	}
	bool operator<( const Self & b) const {
		return std::lexicographical_compare( begin(), end(), 
						     b.begin(), b.end());
	} 
	bool operator==( const Self & b) const { 
		return std::equal( begin(), end(), b.begin()); 
	}
	private:
	Vector vertices;

	template< typename Self, typename Coefficient> 
	friend class ctl::Simplex_boundary< Self, Coefficient>::const_iterator; 
}; //Abstract_simplex
} // namespace ctl

template< typename Stream, typename T>
Stream& operator>>( Stream & in, ctl::Abstract_simplex< T> & simplex){
	T vertex;
	while( in.good()){
		in >> vertex;
		simplex.insert( vertex);	
	}
	return in;
}
	
template< typename Stream, typename T>
Stream& operator<<(Stream& out, const ctl::Abstract_simplex< T> & simplex){
	typedef typename ctl::Abstract_simplex< T>::const_iterator iterator;
	out << "[";
	for(iterator i = simplex.begin(); i != simplex.end(); ++i){
		out << *i;
		if ( i+1 != simplex.end()) { out << ", "; } else { out << "]";}
	}
	return out;
}

#endif // ABSTRACT_SIMPLEX_H
