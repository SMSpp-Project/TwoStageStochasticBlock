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

#include <iostream>
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

 // Test 3: Load from nc4 file if provided
 if( argc > 1 ) {
  cout << "Test 3: Loading from " << argv[ 1 ] << "...";
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
