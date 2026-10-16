/*--------------------------------------------------------------------------*/
/*----------------------------- File test.cpp ------------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Simple test on TwoStageStochasticBlock.
 *
 * \author Benoît Tran \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Benoît Tran
 */
/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "TwoStageStochasticBlock.h"

#include "AbstractBlock.h"

#include "FRealObjective.h"

#include "LinearFunction.h"

#include "PolyhedralFunction.h"

#include <cstdio>
#include <iostream>
#include <set>
#include <typeinfo>

/*--------------------------------------------------------------------------*/
/*-------------------------------- USING -----------------------------------*/
/*--------------------------------------------------------------------------*/

using namespace std;
using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/// Custom terminate function to print the exception message

void smspp_terminate( void ) {
 std::cerr << "Uncaught exception in executing SMS++:\n";
 try {
  std::rethrow_exception( std::current_exception() );
 }
 catch( const std::exception & e ) {
  std::cerr << "\tException type: " << typeid( e ).name() << "\n";
  std::cerr << "\tException message: " << e.what() << "\n";
 } catch( ... ) {
  std::cerr << "\tUnknown exception" << std::endl;
 }
 std::abort();
}

/*--------------------------------------------------------------------------*/
/*----------------------------- TEST HELPERS -------------------------------*/
/*--------------------------------------------------------------------------*/

using Index = Block::Index;

/// a TwoStageStochasticBlock whose scenarios and paths are given in C++

class TestTSSB : public TwoStageStochasticBlock {
 public:
 using TwoStageStochasticBlock::here_and_now_const;

 /// adds a scenario holding n ColVariable x, minimising sum_j x_j if
 /// linear, max_j x_j otherwise; returns x

 std::vector< ColVariable > * add_scenario( Index n , bool linear ) {
  auto b = new AbstractBlock( this );
  auto x = new std::vector< ColVariable >( n );
  b->add_static_variable( *x , "x" );
  Function * f;
  if( linear ) {
   LinearFunction::v_coeff_pair cp;
   for( auto & xj : *x )
    cp.push_back( { & xj , 1.0 } );
   f = new LinearFunction( std::move( cp ) );
   }
  else {
   PolyhedralFunction::VarVector vx;
   PolyhedralFunction::MultiVector A;
   for( Index j = 0 ; j < n ; ++j ) {
    vx.push_back( & ( *x )[ j ] );
    A.push_back( PolyhedralFunction::RealVector( n , 0.0 ) );
    A.back()[ j ] = 1.0;
    }
   f = new PolyhedralFunction( std::move( vx ) , std::move( A ) ,
                               PolyhedralFunction::RealVector( n , 0.0 ) );
   }
  b->set_objective( new FRealObjective( b , f ) , eNoMod );
  v_Block.push_back( b );
  ++f_number_scenarios;
  return( x );
  }

 /// the here-and-now Variable are the given elements of x in each scenario

 void set_here_and_now( std::vector< ColVariable > * x0 ,
                        std::vector< Index > subset ) {
  auto path = new AbstractPath( & ( *x0 )[ 0 ] , v_Block[ 0 ] );
  path->set_last_node_subset( std::move( subset ) );
  v_paths_to_static_vars.emplace_back( path );
  }

 /// the scenarios have the given probabilities

 void set_probabilities( const std::vector< double > & p ) {
  auto dss = new DiscreteScenarioSet();
  dss->load_from_memory( std::vector< std::vector< double > >( p.size() ,
                                                               { 0.0 } ) , p );
  set_scenario_generator( dss );
  }
 };

/*--------------------------------------------------------------------------*/
/// a first-stage path on the non-contiguous subset { 0 , 2 } of x must tie,
/// return, read and write x[ 0 ] and x[ 2 ] of each scenario, and nothing else

bool test_path_subset( void ) {
 const std::vector< Index > subset = { 0 , 2 };
 const Index NS = 3;

 TestTSSB tssb;
 std::vector< std::vector< ColVariable > * > x;
 for( Index s = 0 ; s < NS ; ++s )
  x.push_back( tssb.add_scenario( 4 , true ) );
 tssb.set_here_and_now( x[ 0 ] , subset );

 tssb.generate_abstract_variables();
 tssb.generate_abstract_constraints();

 // the first-stage Variable are x[ 0 ] and x[ 2 ] of scenario 0
 auto fsv = tssb.get_first_stage_variables();
 if( fsv.size() != subset.size() )
  return( false );
 for( Index j = 0 ; j < subset.size() ; ++j )
  if( fsv[ j ] != & ( *x[ 0 ] )[ subset[ j ] ] )
   return( false );

 // the j-th non-anticipativity constraint of each pair of scenarios ties
 // x[ subset[ j ] ] of the two, and nothing else
 const auto & hnc = tssb.here_and_now_const;
 if( ( hnc.shape()[ 0 ] != NS - 1 ) || ( hnc.shape()[ 1 ] != 1 ) )
  return( false );
 std::set< Variable * > tied;
 for( Index t = 0 ; t < NS - 1 ; ++t ) {
  if( hnc[ t ][ 0 ].size() != subset.size() )
   return( false );
  for( Index j = 0 ; j < subset.size() ; ++j ) {
   auto lf = dynamic_cast< LinearFunction * >(
                                       hnc[ t ][ 0 ][ j ].get_function() );
   if( ( ! lf ) || ( lf->get_num_active_var() != 2 ) )
    return( false );
   for( Index k = 0 ; k < 2 ; ++k ) {
    auto v = lf->get_active_var( k );
    bool ok = false;
    for( Index s = 0 ; s < NS ; ++s )
     ok |= ( v == & ( *x[ s ] )[ subset[ j ] ] );
    if( ! ok )
     return( false );
    tied.insert( v );
    }
   }
  }
 if( tied.size() != NS * subset.size() )
  return( false );

 // the Solution reads the values of x[ 0 ] and x[ 2 ] of scenario 0 ...
 for( Index j = 0 ; j < 4 ; ++j )
  ( *x[ 0 ] )[ j ].set_value( j + 1 );
 const char * fn = "TSSB_test_solution.nc4";
 bool ok = true;
 {
  netCDF::NcFile file( fn , netCDF::NcFile::replace );
  auto sol = tssb.get_Solution();
  auto read = file.addGroup( "Read" );
  sol->serialize( read );
  delete sol;
  std::vector< double > han( subset.size() );
  read.getVar( "HereAndNow" ).getVar( han.data() );
  ok = ( han[ 0 ] == 1 ) && ( han[ 1 ] == 3 );

  // ... and writes them back there
  auto write = file.addGroup( "Write" );
  auto dim = write.addDim( "TotalNumberHereAndNow" , subset.size() );
  std::vector< double > val = { 10 , 30 };
  write.addVar( "HereAndNow" , netCDF::NcDouble() , dim ).putVar(
                                                               val.data() );
  TwoStageStochasticBlockSolution wsol;
  wsol.deserialize( write );
  for( auto & xj : *x[ 0 ] )
   xj.set_value( 0 );
  wsol.write( & tssb );
  const std::vector< double > exp = { 10 , 0 , 30 , 0 };
  for( Index j = 0 ; j < 4 ; ++j )
   ok &= ( ( *x[ 0 ] )[ j ].get_value() == exp[ j ] );
 }
 std::remove( fn );

 return( ok );
 }

/*--------------------------------------------------------------------------*/
/*--------------------------------- MAIN -----------------------------------*/
/*--------------------------------------------------------------------------*/

int main( int argc , char ** argv )
{
 // override the default terminate handler to print the exception message
 std::set_terminate( smspp_terminate );

 cout << "=== TwoStageStochasticBlock Factory Test ===" << endl << endl;

 // Test 1: Direct construction
 cout << "Test 1: Direct construction...";
 auto * tssb1 = new TwoStageStochasticBlock();
 if( tssb1 ) {
  cout << " SUCCESS" << endl;
  delete tssb1;
 } else {
  cout << " FAILED" << endl;
  return( 1 );
 }

 // Test 2: Factory registration check
 cout << "Test 2: Factory registration...";
 try {
  auto * tssb2 = dynamic_cast< TwoStageStochasticBlock * >(
   Block::new_Block( "TwoStageStochasticBlock" , nullptr ) );
  if( tssb2 ) {
   cout << " SUCCESS" << endl;
   delete tssb2;
  } else {
   cout << " FAILED: wrong type" << endl;
   return( 1 );
  }
 } catch( const exception & e ) {
  cout << " FAILED: " << e.what() << endl;
  return( 1 );
 }

 // Test 3: first-stage path on a non-contiguous subset
 cout << "Test 3: first-stage path on a subset...";
 if( test_path_subset() )
  cout << " SUCCESS" << endl;
 else {
  cout << " FAILED" << endl;
  return( 1 );
  }

 // Test 4: Load from nc4 file if provided
 if( argc > 1 ) {
  cout << "Test 4: Loading from " << argv[ 1 ] << "...";
  try {
   netCDF::NcFile dataFile( argv[ 1 ] , netCDF::NcFile::read );
   auto * block = Block::new_Block( dataFile , nullptr );
   if( block ) {
    auto * tssb3 = dynamic_cast< TwoStageStochasticBlock * >( block );
    if( tssb3 ) {
     cout << " SUCCESS (scenarios: " << tssb3->get_number_scenarios() << ")" << endl;
    } else {
     cout << " FAILED: not a TwoStageStochasticBlock" << endl;
    }
    delete block;
   } else {
    cout << " FAILED: new_Block returned nullptr" << endl;
    return( 1 );
   }
  } catch( const exception & e ) {
   cout << " FAILED: " << e.what() << endl;
   return( 1 );
  }
 }

 cout << endl << "=== All tests passed ===" << endl;
 return( 0 );
}

/*--------------------------------------------------------------------------*/
/*--------------------------- End File test.cpp ----------------------------*/
/*--------------------------------------------------------------------------*/
