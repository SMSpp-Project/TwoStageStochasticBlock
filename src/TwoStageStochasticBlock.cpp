/*--------------------------------------------------------------------------*/
/*-------------------- File TwoStageStochasticBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Implementation of the TwoStageStochasticBlock class.
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*---------------------------- IMPLEMENTATION ------------------------------*/
/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "TwoStageStochasticBlock.h"
#include "DataMapping.h"
#include <iostream>
#include <cstdlib>  // for std::rand()
#include <cstdio>   // for std::remove()

/*--------------------------------------------------------------------------*/
/*------------------------- NAMESPACE AND USING ----------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*----------------------------- STATIC MEMBERS -----------------------------*/
/*--------------------------------------------------------------------------*/

SMSpp_insert_in_factory_cpp_1( TwoStageStochasticBlock );

/*--------------------------------------------------------------------------*/
/*-------------------- METHODS of TwoStageStochasticBlock ------------------*/
/*--------------------------------------------------------------------------*/

TwoStageStochasticBlock::~TwoStageStochasticBlock() {

 Constraint::clear( here_and_now_const );

 for( auto & block : v_Block )
  delete( block );
 v_Block.clear();
}

/*--------------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::generate_abstract_variables( Configuration * stvv )
{
 if( variables_generated() )  // variables have already been generated
  return;                     // nothing to do

 // Call base class to generate variables in all sub-blocks
 Block::generate_abstract_variables( stvv );

 // Mark variables as generated
 set_variables_generated();

}  // end( TwoStageStochasticBlock::generate_abstract_variables )

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::generate_abstract_constraints( Configuration * stcc )
{
 if( constraints_generated() )  // constraints have already been generated
  return;                       // nothing to do

 // Ensure variables are generated first
 if( ! variables_generated() )
  generate_abstract_variables();

 bool gen_seq_anchr_cnstrs = true; // sequential by default
 if( ( ! stcc ) && f_BlockConfig )
  stcc = f_BlockConfig->f_static_constraints_Configuration;
 if( auto sci = dynamic_cast< SimpleConfiguration< int > * >( stcc ) )
  gen_seq_anchr_cnstrs = sci->f_value;

 // Precompute variables for each scenario t, path i, and variable j
 boost::multi_array< std::vector< ColVariable * > , 2 > here_and_now_vars;
 here_and_now_vars.resize(
  boost::extents[ get_number_scenarios() ][ v_paths_to_static_vars.size() ] );

 for( int t = 0 ; t < get_number_scenarios() ; ++t ) {
  auto block = get_sub_Block( t );
  block->generate_abstract_constraints( stcc );
  for( int i = 0 ; i < v_paths_to_static_vars.size() ; ++i ) {
   auto number_variables = v_paths_to_static_vars[ i ]->
    get_number_elements< ColVariable >( block );
   here_and_now_vars[ t ][ i ].resize( number_variables );
   // pointer to the first ColVariable of a contiguous structure,
   // i.e., a std::vector or boost::multi_array (since *static* vars)
   auto * elem = v_paths_to_static_vars[ i ]->
    get_element< ColVariable >( block );
   for( int j = 0 ; j < number_variables ; j++ )
    here_and_now_vars[ t ][ i ][ j ] = elem + j;
  }
 }

 here_and_now_const.resize(
  boost::extents[ get_number_scenarios() - 1 ][ v_paths_to_static_vars.size() ] );

 LinearFunction::v_coeff_pair vars;

 if( gen_seq_anchr_cnstrs ) { // sequential constraints

  // loop through each scenario, except the last one
  // scenario_0 = scenario_1
  // scenario_1 = scenario_2
  // . . .
  // scenario_i-1 = scenario_i
  for( int t = 0 ; t < get_number_scenarios() - 1 ; ++t ) {

   // loop through each here-and-now variable
   for( int i = 0 ; i < v_paths_to_static_vars.size() ; ++i ) {

    here_and_now_const[ t ][ i ].resize( here_and_now_vars[ t ][ i ].size() );
    for( int j = 0 ; j < here_and_now_vars[ t ][ i ].size() ; j++ ) {
     // create a constraint that ensures the j-th variable at the t-th scenario...
     vars.push_back( std::make_pair( here_and_now_vars[ t ][ i ][ j ] , 1.0 ) );
     // ... is equal to the same j-th variable at (t+1)-th scenario
     vars.push_back( std::make_pair( here_and_now_vars[ t + 1 ][ i ][ j ] , -1.0 ) );

     here_and_now_const[ t ][ i ][ j ].set_both( 0.0 );
     here_and_now_const[ t ][ i ][ j ].set_function(
      new LinearFunction( std::move( vars ) ) );
    }
   }
  }

 } else { // anchor constraints

  // loop through each scenario, starting from the second one
  // scenario_0 = scenario_1
  // scenario_0 = scenario_2
  // . . .
  // scenario_0 = scenario_i
  for( int t = 1 ; t < get_number_scenarios() ; ++t ) {

   // loop through each here-and-now variable
   for( int i = 0 ; i < v_paths_to_static_vars.size() ; ++i ) {

    here_and_now_const[ t - 1 ][ i ].resize( here_and_now_vars[ t ][ i ].size() );
    for( int j = 0 ; j < here_and_now_vars[ t ][ i ].size() ; j++ ) {
     // create a constraint that ensures the j-th variable at the first scenario...
     vars.push_back( std::make_pair( here_and_now_vars[ 0 ][ i ][ j ] , 1.0 ) );
     // ... is equal to the same j-th variable at t-th scenario
     vars.push_back( std::make_pair( here_and_now_vars[ t ][ i ][ j ] , -1.0 ) );

     here_and_now_const[ t - 1 ][ i ][ j ].set_both( 0.0 );
     here_and_now_const[ t - 1 ][ i ][ j ].set_function(
      new LinearFunction( std::move( vars ) ) );
    }
   }
  }
 }

 add_static_constraint( here_and_now_const , "Here_and_Now_Const" );

 set_constraints_generated();
} // end( TwoStageStochasticBlock::generate_abstract_constraints )

/*
 generate_objective() is not overridden currently - MILPSolver handles 
 objective
aggregation automatically through its breadth-first traversal of all
nested blocks, resulting in an unweighted sum of scenario objectives.
So for solving it with the extensive form, it is ok for now.
/* 

/*--------------------------------------------------------------------------*/
/*-------------------- Methods for handling Modification -------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::add_Modification( sp_Mod mod ,
                                                Observer::ChnlName chnl ) {
 // TODO
 if( anyone_there() )
  Block::add_Modification( std::make_shared< NBModification >( this ) , chnl );
}



/*--------------------------------------------------------------------------*/
/*------ METHODS FOR READING THE DATA OF THE TwoStageStochasticBlock -------*/
/*--------------------------------------------------------------------------*/

int TwoStageStochasticBlock::get_objective_sense() const {
 try {
  auto sub_Block = get_sub_Block( 0 );
  if( sub_Block )
   return( sub_Block->get_objective_sense() );
 }
 catch( ... ) {}
 return( Objective::eUndef );
}

/*--------------------------------------------------------------------------*/

Block * TwoStageStochasticBlock::get_sub_Block( Index scenario ) const {
 if( scenario >= get_number_scenarios() )
  throw( std::invalid_argument( "TwoStageStochasticBlock::get_sub_Block: invalid scenario " +
                                std::to_string( scenario ) ) );
 return( v_Block[ scenario ] );
}

/*--------------------------------------------------------------------------*/

std::vector< ColVariable * > TwoStageStochasticBlock::get_first_stage_variables() const {
 std::vector< ColVariable * > first_stage_vars;
 
 // Check if variables have been generated
 if( !variables_generated() || v_Block.empty() || v_paths_to_static_vars.empty() ) {
  return first_stage_vars;  // Return empty vector
 }
 
 // Get the first scenario block (all scenarios have the same first-stage variables
 // due to non-anticipativity constraints)
 Block* scenario_0_block = v_Block[0];
 if( !scenario_0_block ) {
  return first_stage_vars;  // Return empty vector
 }
 
 // Iterate through all AbstractPaths to static (first-stage) variables
 for( const auto& path : v_paths_to_static_vars ) {
  // Get the number of variables this path points to
  auto num_vars = path->get_number_elements< ColVariable >( scenario_0_block );
  
  // Get pointer to the first variable in the contiguous structure
  auto* first_var = path->get_element< ColVariable >( scenario_0_block );
  
  // Add all variables from this path to our result vector
  for( Index j = 0; j < num_vars; ++j ) {
   first_stage_vars.push_back( first_var + j );
  }
 }
 
 return first_stage_vars;
}

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::apply_scenario_data( 
    Index scenario_index, 
    const std::vector<double>& scenario_data,
    c_ModParam issuePMod,
    c_ModParam issueAMod ) {
 
 if( scenario_index >= f_number_scenarios )
  throw std::invalid_argument( 
   "TwoStageStochasticBlock::apply_scenario_data: invalid scenario index " +
   std::to_string(scenario_index) );
 
 if( !stochastic_block )
  throw std::logic_error(
   "TwoStageStochasticBlock::apply_scenario_data: no StochasticBlock available" );
 
 // Get the block copy for this scenario
 Block* scenario_block = v_Block[scenario_index];
 
 if( !scenario_block ) {
  throw std::logic_error(
   "TwoStageStochasticBlock::apply_scenario_data: no block available for scenario " +
   std::to_string(scenario_index) );
 }
 
 // Use the local apply_scenario_to_block() method to apply scenario data
 // to the specific scenario block copy
 apply_scenario_to_block( scenario_block, scenario_data, issuePMod, issueAMod );
}

/*--------------------------------------------------------------------------*/
/*----- METHODS DESCRIBING THE BEHAVIOR OF AN TwoStageStochasticBlock ------*/
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/*------- METHODS FOR PRINTING & SAVING THE TwoStageStochasticBlock --------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::print( std::ostream & output , char vlvl ) const
{
 output << std::endl << "TwoStageStochasticBlock with ";

 if( v_Block.empty() )
  output << "no inner Block";
 else
  output << v_Block.size() << " sub-Block" << std::endl;
}

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::serialize( netCDF::NcGroup & group ) const
{
 Block::serialize( group );

 // type
 group.putAtt( "type" , "TwoStageStochasticBlock" );

 // StochasticBlock
 auto sub_group = group.addGroup( "StochasticBlock" );
 get_sub_Block( 0 )->serialize( sub_group );

 // Scenarios

 // scenario_set.serialize( group );

}

/*--------------------------------------------------------------------------*/
/*-------------------------- PRIVATE METHODS -------------------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::apply_scenario_to_block( 
    Block * target_block,
    const std::vector< double > & scenario_data,
    c_ModParam issuePMod,
    c_ModParam issueAMod ) {
 
 if( !stochastic_block )
  throw std::logic_error(
   "TwoStageStochasticBlock::apply_scenario_to_block: no StochasticBlock available" );
 
 // Get the DataMappings from the StochasticBlock
 const auto& data_mappings = stochastic_block->get_data_mappings();
 
 // Save the original caller (the inner block of StochasticBlock)
 Block* original_caller = stochastic_block->get_inner_block();
 
 // Apply each mapping to the target block
 for( auto & mapping : data_mappings ) {
  mapping->set_caller( target_block );
  mapping->set_data( scenario_data.begin(), issuePMod, issueAMod );
 }
 
 // Restore the original caller for all mappings
 for( auto & mapping : data_mappings ) {
  mapping->set_caller( original_caller );
 }
}

/*--------------------------------------------------------------------------*/
/*---------------- End File TwoStageStochasticBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
