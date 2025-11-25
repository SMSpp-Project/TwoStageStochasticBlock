/*--------------------------------------------------------------------------*/
/*----------------------- File test_factory.cpp ----------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Simple test to verify TwoStageStochasticBlock factory registration.
 */
/*--------------------------------------------------------------------------*/

#include "TwoStageStochasticBlock.h"
#include <iostream>
#include <netcdf>

using namespace std;
using namespace SMSpp_di_unipi_it;

int main( int argc , char ** argv ) {

 cout << "=== TwoStageStochasticBlock Factory Test ===" << endl << endl;

 // Test 1: Direct construction
 cout << "Test 1: Direct construction..." << endl;
 auto * tssb1 = new TwoStageStochasticBlock();
 if( tssb1 ) {
  cout << "  SUCCESS: Created TwoStageStochasticBlock directly" << endl;
  delete tssb1;
 } else {
  cout << "  FAILED: Could not create TwoStageStochasticBlock" << endl;
  return 1;
 }

 // Test 2: Factory registration check
 cout << endl << "Test 2: Check if class is registered in factory..." << endl;
 try {
  auto * tssb2 = dynamic_cast< TwoStageStochasticBlock * >(
   Block::new_Block( "TwoStageStochasticBlock" , nullptr ) );
  if( tssb2 ) {
   cout << "  SUCCESS: Factory can create TwoStageStochasticBlock" << endl;
   delete tssb2;
  } else {
   cout << "  FAILED: Factory returned wrong type" << endl;
   return 1;
  }
 } catch( const exception & e ) {
  cout << "  FAILED: Exception: " << e.what() << endl;
  return 1;
 }

 // Test 3: Try loading from nc4 file if provided
 if( argc > 1 ) {
  cout << endl << "Test 3: Loading from file " << argv[1] << "..." << endl;

  try {
   netCDF::NcFile dataFile( argv[1] , netCDF::NcFile::read );

   // Check classname attribute
   try {
    auto classname_attr = dataFile.getAtt( "classname" );
    if( ! classname_attr.isNull() ) {
     string classname;
     classname_attr.getValues( classname );
     cout << "  File classname: '" << classname << "'" << endl;
    } else {
     cout << "  No classname attribute in file" << endl;
    }
   } catch( const exception & e ) {
    cout << "  Could not read classname: " << e.what() << endl;
   }

   // Try to deserialize
   auto * block = Block::new_Block( dataFile , nullptr );
   if( block ) {
    cout << "  SUCCESS: Loaded block from file" << endl;
    cout << "  Block type: " << typeid(*block).name() << endl;

    auto * tssb3 = dynamic_cast< TwoStageStochasticBlock * >( block );
    if( tssb3 ) {
     cout << "  Block IS a TwoStageStochasticBlock" << endl;
     cout << "  Number of scenarios: " << tssb3->get_number_scenarios() << endl;
    } else {
     cout << "  Block is NOT a TwoStageStochasticBlock" << endl;
    }

    delete block;
   } else {
    cout << "  FAILED: Block::new_Block returned nullptr" << endl;
    return 1;
   }

  } catch( const exception & e ) {
   cout << "  FAILED: Exception: " << e.what() << endl;
   return 1;
  }
 }

 cout << endl << "=== All tests passed ===" << endl;
 return 0;
}

/*--------------------------------------------------------------------------*/
/*---------------------- End File test_factory.cpp -------------------------*/
/*--------------------------------------------------------------------------*/
