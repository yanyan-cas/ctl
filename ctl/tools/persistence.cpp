/*******************************************************************************
* -Academic Honesty-
* Plagarism: The unauthorized use or close imitation of the language and 
* thoughts of another author and the representation of them as one's own 
* original work, as by not crediting the author. 
* (Encyclopedia Britannica, 2008.)
*
* You are free to use the code according to the license below, but, please
* do not commit acts of academic dishonesty. We strongly encourage and request 
* that for any [academic] use of this source code one should cite one the 
* following works:
* 
* \cite{hatcher, z-ct-10}
* 
* See ct.bib for the corresponding bibtex entries. 
* !!! DO NOT CITE THE USER MANUAL !!!
*******************************************************************************
* Copyright (C) Ryan H. Lewis 2014 <me@ryanlewis.net>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program in a file entitled COPYING; if not, write to the 
* Free Software Foundation, Inc., 
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************************
*******************************************************************************/
//CTL Headers

//Utility 
#include <ctl/io/io.h>
#include <ctl/utility/timer.h>

//#define COMPUTE_BETTI
//#define CTL_USE_MURMUR
#define CTL_USE_CITY
//#define CTL_USE_JENKINS
//#define COMPLEX_DIAGNOSTICS
//#define ZOOM_PROFILE
#ifdef ZOOM_PROFILE
#include "zoom.h"
#endif

//CTL 
//Types for Building a Simplicial Chain Complex and Filtration
//Abstract Simplex
#include <ctl/finite_field/finite_field.h>
#include <ctl/abstract_simplex/abstract_simplex.h>
#include <ctl/abstract_simplex/simplex_boundary.h>

//Filtration & Filtration Boundary Operator
#include <ctl/filtration/filtration.h>
#include <ctl/filtration/filtration_boundary.h>
#include <ctl/filtration/less.h>

//Weights
#include <ctl/weight_data/weight_data.h>
#include <ctl/weight_data/weight_functor.h>

//Chaim Complex
#include <ctl/chain_complex/chain_complex.h>
#include <ctl/chain_complex/complex_boundary.h>
#include <ctl/term/term.h>

//Chains & Persistence 
#include <ctl/chain/chain.h>
#include <ctl/persistence/persistence.h>
#include <ctl/persistence/iterator_property_map.h>
#include <ctl/persistence/offset_maps.h>
#include <ctl/persistence/compute_betti.h>
#include <ctl/persistence/compute_barcodes.h>

//Barcodes
#include <ctl/barcodes/barcodes.h>

//STL
#include <sstream>


//Timer
typedef ctl::Timer Timer;

//Simplex
typedef ctl::Abstract_simplex< int> Simplex;
typedef ctl::Finite_field< 2> Z2;
typedef ctl::Simplex_boundary< Simplex, Z2> Simplex_boundary;

//Chain Complex
typedef ctl::Chain_complex< Simplex, Simplex_boundary> Complex;

//Chain Complex with weights
typedef ctl::Weight_data< double> Weight_data;
typedef ctl::Chain_complex< Simplex, Simplex_boundary, Weight_data> 
							Weighted_complex;

//Filtration order
typedef ctl::Cell_less Complex_cell_less;
typedef ctl::Weight_less Complex_weight_less;

template< typename Complex, typename Cell_less, 
	  typename Barcode, typename Timer, 
	  typename Tag>
void run_persistence( Complex & complex, 
		      Cell_less & less, 
		      Barcode & barcode,
 		      Timer & timer,
		      const Tag tag){
   typedef ctl::Filtration< Complex, Cell_less > Filtration;
   //Boundary Operator
   //NOTE: This is not a general purpose boundary operator.
   //It works correctly only when successive 
   //boundaries are taken in a filtration order
   //typedef typename Filtration::iterator Filtration_iterator;
   typedef ctl::Filtration_boundary< Filtration> Filtration_boundary;
   typedef typename Filtration::Term Filtration_term;
   
   //Chain Type
   typedef ctl::Chain< Filtration_term> Complex_chain;
   
   //Essentially our sparse boundary matrix
   typedef std::vector< Complex_chain> Complex_chains;
   
   //Generic wrapper to access the sparse matrix.
   typedef ctl::Pos_offset_map< typename Filtration_term::Cell> 
						Complex_offset_map;
   typedef ctl::iterator_property_map< typename Complex_chains::iterator, 
                                         Complex_offset_map, 
                                         Complex_chain, 
                                         Complex_chain&> Complex_chain_map;
   //produce a filtration
   timer.start();
   Filtration complex_filtration( complex);
   Filtration_boundary filtration_boundary( complex_filtration);
   timer.stop();
   double complex_filtration_time = timer.elapsed();
   //display some info
   std::cout << "time to compute complex filtration (secs): " 
             << complex_filtration_time << std::endl;
   
   //begin instantiate our vector of cascades homology
   Complex_chains complex_cascade_boundaries( complex.size(), Complex_chain());
   
   Complex_offset_map offset_map( complex_filtration.begin());
   //we hand persistence a property map for genericity!                                         
   Complex_chain_map cascade_bd_property_map( complex_cascade_boundaries.begin(),
         				     offset_map);
   //serial persistence (complex)
   timer.start();
   ctl::persistence( complex_filtration.begin(), complex_filtration.end(),
   		    filtration_boundary, cascade_bd_property_map);
   timer.stop();
   
   double complex_persistence = timer.elapsed();
   std::cout << "serial persistence: " << complex_persistence << std::endl;
   ctl::compute_barcodes( complex_filtration, 
			  cascade_bd_property_map, 
			  barcode, tag);
}

template<typename String>
void process_args(int & argc, char *argv[], 
		  String & filename, String & filtration){
  std::string usage = "Usage: persistence input-space.asc [filtration.flt]";
  if (argc != 2 && argc != 3){ std::cerr << usage << std::endl; exit( 1); }
  filename = std::string( argv[ 1]);
  if( argc == 2){
    filtration = filename;
    std::size_t pos = filtration.rfind('.');
    if (pos != std::string::npos){ filtration.erase(pos); }
    filtration.append(".flt");
    return;
  }else { filtration = std::string( argv[ 2]); }
  std::cout << "filename: " << filename << std::endl;
  std::cout << "filtration filename: " << filtration << std::endl;

}

template< typename String, typename Weight_functor>
bool read_weights( const String & filename, Weight_functor & f){
	std::ifstream in;
	if( !ctl::open_file( in, filename.c_str())){ return false; }
	in >> f;
	return true;
}

int main(int argc, char *argv[]){
  std::string full_complex_name, filtration_file;
  process_args(argc, argv, full_complex_name, filtration_file);
  
  std::string barcode_file_name( full_complex_name); 
  std::size_t pos = barcode_file_name.rfind( '.');
  if( pos != std::string::npos){
	barcode_file_name.replace(pos, barcode_file_name.length(), ".bcs");
  }
  std::ofstream out; 
  ctl::open_file( out, barcode_file_name.c_str());
  //Create some data structures 
  Timer timer;
  ctl::Weight_data_functor< Weighted_complex> weight_functor;
  timer.start();
  if( read_weights( filtration_file, weight_functor) ){
	typedef typename ctl::Barcodes< double> Barcodes;
	Barcodes barcodes;
  	Weighted_complex complex;
  	ctl::read_complex( full_complex_name, complex, weight_functor); 
  	timer.stop();
  	std::cout << "I/O Time: " << timer.elapsed() << std::endl;
  	std::cout << "complex size: " << complex.size() << std::endl; 
  	std::cout << "complex dimension: " << complex.dimension() << std::endl;
	Complex_weight_less less;
	run_persistence( complex, less, barcodes, timer, ctl::detail::weighted_tag());
	out << barcodes << std::endl;

  }else{
	typedef typename ctl::Barcodes< std::size_t> Barcodes;
	Barcodes barcodes;
  	Complex complex;
  	ctl::read_complex( full_complex_name, complex);
  	timer.stop();
  	std::cout << "I/O Time: " << timer.elapsed() << std::endl;
	std::cout << "complex size: " << complex.size() << std::endl; 
  	std::cout << "complex dimension: " << complex.dimension() << std::endl;
	Complex_cell_less less;
  	run_persistence( complex, less, barcodes, timer, ctl::detail::non_weighted_tag());   
	out << barcodes << std::endl;
  } 
  
  return 0;
}
