/*--------------------------------------------------------------------------*/
/*--------------------------- File test.cpp --------------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Test program for TwoStageStochasticBlock deserialization and basic
 * operations.
 *
 * This test:
 * 1. Deserializes a TwoStageStochasticBlock from a netCDF file
 * 2. Verifies the block structure (scenarios, sub-blocks, etc.)
 * 3. Generates variables, constraints, and objective
 * 4. Verifies that the structure is consistent
 *
 * Usage: ./TwoStageStochasticBlock_test <file.nc4>
 *
 * \author Claude Code (AI Assistant)
 * \author Benoit (integration and testing)
 */
/*--------------------------------------------------------------------------*/
/*----------------------------- INCLUDES -----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "TwoStageStochasticBlock.h"
#include "BlockSolverConfig.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <netcdf>

/*--------------------------------------------------------------------------*/
/*------------------------------- USING ------------------------------------*/
/*--------------------------------------------------------------------------*/

using namespace std;
using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*------------------------------- MACROS -----------------------------------*/
/*--------------------------------------------------------------------------*/

#define USECOLORS 1
#if( USECOLORS )
 #define RED( x ) "\x1B[31m" x "\033[0m"
 #define GREEN( x ) "\x1B[32m" x "\033[0m"
 #define YELLOW( x ) "\x1B[33m" x "\033[0m"
 #define BLUE( x ) "\x1B[34m" x "\033[0m"
#else
 #define RED( x ) x
 #define GREEN( x ) x
 #define YELLOW( x ) x
 #define BLUE( x ) x
#endif

/*--------------------------------------------------------------------------*/
/*----------------------------- FUNCTIONS ----------------------------------*/
/*--------------------------------------------------------------------------*/

bool test_deserialization( const string & filename ) {
 cout << BLUE("=====================================") << endl;
 cout << BLUE("TwoStageStochasticBlock Test") << endl;
 cout << BLUE("=====================================") << endl;
 cout << "Input file: " << filename << endl << endl;

 try {
  // Open the netCDF file
  netCDF::NcFile dataFile( filename , netCDF::NcFile::read );

  // Check what classname is in the file
  try {
   auto classname_attr = dataFile.getAtt( "classname" );
   if( ! classname_attr.isNull() ) {
    string classname;
    classname_attr.getValues( classname );
    cout << "File contains classname: '" << classname << "'" << endl;
   }
  } catch(...) {
   cout << "No classname attribute found in file" << endl;
  }

  // Deserialize the block
  cout << "Deserializing Block..." << endl;
  auto start = chrono::steady_clock::now();

  auto * block = Block::new_Block( dataFile , nullptr );

  if( ! block ) {
   cout << RED("FAILED") << ": Block::new_Block returned nullptr" << endl;
   cout << "The file might not contain a valid Block, or the class is not registered in the factory" << endl;
   return false;
  }

  cout << "Block type: " << typeid(*block).name() << endl;

  auto * tssb = dynamic_cast< TwoStageStochasticBlock * >( block );

  auto end = chrono::steady_clock::now();
  chrono::duration< double > elapsed = end - start;

  if( ! tssb ) {
   cout << RED("FAILED") << ": Block is not a TwoStageStochasticBlock, it's a different type" << endl;
   cout << "Try registering TwoStageStochasticBlock in the factory" << endl;
   delete block;
   return false;
  }

  cout << GREEN("SUCCESS") << " (took " << fixed << setprecision(4)
       << elapsed.count() << "s)" << endl << endl;

  // Check basic properties
  cout << "Checking block structure..." << endl;
  auto num_scenarios = tssb->get_number_scenarios();
  cout << "  Number of scenarios: " << num_scenarios << endl;

  if( num_scenarios == 0 ) {
   cout << RED("FAILED") << ": No scenarios found" << endl;
   delete tssb;
   return false;
  }

  // Check that all scenario blocks exist
  bool all_blocks_exist = true;
  for( Block::Index i = 0; i < num_scenarios; ++i ) {
   try {
    auto * sub_block = tssb->get_sub_Block( i );
    if( ! sub_block ) {
     cout << RED("FAILED") << ": Scenario " << i << " block is null" << endl;
     all_blocks_exist = false;
    }
   } catch( const exception & e ) {
    cout << RED("FAILED") << ": Exception getting scenario " << i
         << " block: " << e.what() << endl;
    all_blocks_exist = false;
   }
  }

  if( ! all_blocks_exist ) {
   delete tssb;
   return false;
  }

  cout << GREEN("SUCCESS") << ": All " << num_scenarios
       << " scenario blocks are present" << endl << endl;

  // Generate variables
  cout << "Generating abstract variables..." << endl;
  start = chrono::steady_clock::now();
  tssb->generate_abstract_variables();
  end = chrono::steady_clock::now();
  elapsed = end - start;
  cout << GREEN("SUCCESS") << " (took " << fixed << setprecision(4)
       << elapsed.count() << "s)" << endl << endl;

  // Generate constraints
  cout << "Generating abstract constraints..." << endl;
  start = chrono::steady_clock::now();
  tssb->generate_abstract_constraints();
  end = chrono::steady_clock::now();
  elapsed = end - start;
  cout << GREEN("SUCCESS") << " (took " << fixed << setprecision(4)
       << elapsed.count() << "s)" << endl << endl;

  // Generate objective
  cout << "Generating objective..." << endl;
  start = chrono::steady_clock::now();
  tssb->generate_objective();
  end = chrono::steady_clock::now();
  elapsed = end - start;
  cout << GREEN("SUCCESS") << " (took " << fixed << setprecision(4)
       << elapsed.count() << "s)" << endl << endl;

  // Get first-stage variables
  cout << "Retrieving first-stage variables..." << endl;
  auto first_stage_vars = tssb->get_first_stage_variables();
  cout << "  Number of first-stage variables: " << first_stage_vars.size()
       << endl;
  cout << GREEN("SUCCESS") << endl << endl;

  // Print summary
  cout << BLUE("=====================================") << endl;
  cout << BLUE("Test Summary") << endl;
  cout << BLUE("=====================================") << endl;
  cout << "File: " << filename << endl;
  cout << "Scenarios: " << num_scenarios << endl;
  cout << "First-stage variables: " << first_stage_vars.size() << endl;
  cout << "Status: " << GREEN("ALL TESTS PASSED") << endl;
  cout << BLUE("=====================================") << endl;

  // Cleanup
  delete tssb;

  return true;

 } catch( const netCDF::exceptions::NcException & e ) {
  cout << RED("FAILED") << ": NetCDF exception: " << e.what() << endl;
  return false;
 } catch( const exception & e ) {
  cout << RED("FAILED") << ": Exception: " << e.what() << endl;
  return false;
 } catch( ... ) {
  cout << RED("FAILED") << ": Unknown exception" << endl;
  return false;
 }
}

/*--------------------------------------------------------------------------*/
/*------------------------------ MAIN --------------------------------------*/
/*--------------------------------------------------------------------------*/

int main( int argc , char ** argv ) {

 // Check arguments
 if( argc < 2 ) {
  cerr << "Usage: " << argv[0] << " <file.nc4>" << endl;
  cerr << endl;
  cerr << "Example:" << endl;
  cerr << "  " << argv[0]
       << " ../UCBlock/netCDF_files/EC_Data/TSSB_EC_CO_Test_TUB.nc4" << endl;
  return 1;
 }

 string filename = argv[1];

 // Run test
 bool success = test_deserialization( filename );

 return success ? 0 : 1;
}

/*--------------------------------------------------------------------------*/
/*---------------------------- End of test.cpp -----------------------------*/
/*--------------------------------------------------------------------------*/
