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
#include "FRealObjective.h"
#include "LinearFunction.h"
#include <iostream>

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

TwoStageStochasticBlock::~TwoStageStochasticBlock( ) {
 Constraint::clear( here_and_now_const );

 for(auto & block : v_Block)
  delete (block);
 v_Block.clear( );

 if( scenario_generator ) delete scenario_generator;

 if( stochastic_block ) delete stochastic_block;
}

/*--------------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::generate_abstract_variables( Configuration * stvv
  ) {
 if( variables_generated( )) // variables have already been generated
  return;                   // nothing to do

 // Call base class to generate variables in all sub-blocks
 Block::generate_abstract_variables( stvv );

 // Mark variables as generated
 set_variables_generated( );

} // end( TwoStageStochasticBlock::generate_abstract_variables )

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::generate_abstract_constraints(
  Configuration * stcc ) {
 if( constraints_generated( )) // constraints have already been generated
  return;                     // nothing to do

 // Ensure variables are generated first
 if( ! variables_generated( )) generate_abstract_variables( );

 bool gen_seq_anchr_cnstrs = true; // sequential by default
 if((! stcc) && f_BlockConfig )
  stcc = f_BlockConfig->f_static_constraints_Configuration;
 if( auto sci = dynamic_cast< SimpleConfiguration< int > * >(stcc))
  gen_seq_anchr_cnstrs = sci->f_value;

 // Precompute variables for each scenario t, path i, and variable j
 boost::multi_array< std::vector< ColVariable * > , 2 > here_and_now_vars;
 here_and_now_vars.resize(
   boost::extents[ get_number_scenarios( ) ][ v_paths_to_static_vars.size( ) ] )
 ;

 for(int t = 0; t < get_number_scenarios( ); ++t) {
  auto block = get_sub_Block( t );
  block->generate_abstract_constraints( stcc );
  for(int i = 0; i < v_paths_to_static_vars.size( ); ++i) {
   auto number_variables =
     v_paths_to_static_vars[ i ]->get_number_elements< ColVariable >( block );
   here_and_now_vars[ t ][ i ].resize( number_variables );
   // pointer to the first ColVariable of a contiguous structure,
   // i.e., a std::vector or boost::multi_array (since *static* vars)
   auto * elem = v_paths_to_static_vars[ i ]->get_element< ColVariable >( block
    );
   for(int j = 0; j < number_variables; j++)
    here_and_now_vars[ t ][ i ][ j ] = elem + j;
  }
 }

 here_and_now_const.resize(
   boost::extents[ get_number_scenarios( ) - 1 ][ v_paths_to_static_vars.size( )
   ] );

 LinearFunction::v_coeff_pair vars;

 if( gen_seq_anchr_cnstrs ) { // sequential constraints

  // loop through each scenario, except the last one
  // scenario_0 = scenario_1
  // scenario_1 = scenario_2
  // . . .
  // scenario_i-1 = scenario_i
  for(int t = 0; t < get_number_scenarios( ) - 1; ++t) {
   // loop through each here-and-now variable
   for(int i = 0; i < v_paths_to_static_vars.size( ); ++i) {
    here_and_now_const[ t ][ i ].resize( here_and_now_vars[ t ][ i ].size( ));
    for(int j = 0; j < here_and_now_vars[ t ][ i ].size( ); j++) {
     // create a constraint that ensures the j-th variable at the t-th
     // scenario...
     vars.push_back( std::make_pair( here_and_now_vars[ t ][ i ][ j ] , 1.0 ));
     // ... is equal to the same j-th variable at (t+1)-th scenario
     vars.push_back( std::make_pair( here_and_now_vars[ t + 1 ][ i ][ j ] , -1.0
      ));

     here_and_now_const[ t ][ i ][ j ].set_both( 0.0 );
     here_and_now_const[ t ][ i ][ j ].set_function(
       new LinearFunction( std::move( vars )));
    }
   }
  }

 }
 else {   // anchor constraints

  // loop through each scenario, starting from the second one
  // scenario_0 = scenario_1
  // scenario_0 = scenario_2
  // . . .
  // scenario_0 = scenario_i
  for(int t = 1; t < get_number_scenarios( ); ++t) {
   // loop through each here-and-now variable
   for(int i = 0; i < v_paths_to_static_vars.size( ); ++i) {
    here_and_now_const[ t - 1 ][ i ].resize( here_and_now_vars[ t ][ i ].size( )
      );
    for(int j = 0; j < here_and_now_vars[ t ][ i ].size( ); j++) {
     // create a constraint that ensures the j-th variable at the first
     // scenario...
     vars.push_back( std::make_pair( here_and_now_vars[ 0 ][ i ][ j ] , 1.0 ));
     // ... is equal to the same j-th variable at t-th scenario
     vars.push_back( std::make_pair( here_and_now_vars[ t ][ i ][ j ] , -1.0 ));

     here_and_now_const[ t - 1 ][ i ][ j ].set_both( 0.0 );
     here_and_now_const[ t - 1 ][ i ][ j ].set_function(
       new LinearFunction( std::move( vars )));
    }
   }
  }
 }

 add_static_constraint( here_and_now_const , "Here_and_Now_Const" );

 set_constraints_generated( );
} // end( TwoStageStochasticBlock::generate_abstract_constraints )

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::generate_objective( Configuration * objc ) {
 if( objective_generated( )) // objective has already been generated
  return;                   // nothing to do

 // First, generate objectives for all scenario blocks
 for(auto * block : v_Block) {
  if( block ) block->generate_objective( objc );
 }

 // Scale each scenario block's objective by its probability
 if( scenario_generator && ! v_Block.empty( )) {
  // Check if pool is already initialized to avoid re-initialization overhead
  if( scenario_generator->is_pool_initialized( )) {
   // Reset to beginning of existing pool
   scenario_generator->reset_pool( );
  }
  else {
   // Initialize the scenario generator to iterate through all scenarios
   scenario_generator->init_representative_pool( f_number_scenarios );
  }

  Index scenario_idx = 0;

  // Use do-while pattern to iterate through scenarios
  do {
   if( scenario_idx < v_Block.size( ) && v_Block[ scenario_idx ] ) {
    // Get probability for current scenario
    double prob = scenario_generator->get_current_scenario_probability( );

    // Scale this scenario's objective
    scale_scenario_objective( v_Block[ scenario_idx ] , prob );
   }
   scenario_idx++;
  } while( scenario_generator->next_scenario( ));
 }
 // Note: If no scenario_generator, objectives remain unscaled (implicitly
 // equal weights)

 // Now call base class to aggregate the (already scaled) objectives
 Block::generate_objective( objc );

 set_objective_generated( );

} // end( TwoStageStochasticBlock::generate_objective )

/*--------------------------------------------------------------------------*/
/*-------------------- Methods for handling Modification -------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::add_Modification(
  sp_Mod mod ,
  Observer::ChnlName chnl ) {
 // TODO
 if( anyone_there( ))
  Block::add_Modification( std::make_shared< NBModification >( this ) , chnl );
}

/*--------------------------------------------------------------------------*/
/*------ METHODS FOR READING THE DATA OF THE TwoStageStochasticBlock -------*/
/*--------------------------------------------------------------------------*/

int TwoStageStochasticBlock::get_objective_sense( ) const {
 try {
  auto sub_Block = get_sub_Block( 0 );
  if( sub_Block ) return(sub_Block->get_objective_sense( ));
 } catch( ... ) {}
 return(Objective::eUndef);
}

/*--------------------------------------------------------------------------*/

Block *TwoStageStochasticBlock::get_sub_Block( Index scenario ) const {
 if( scenario >= get_number_scenarios( ))
  throw(std::invalid_argument(
   "TwoStageStochasticBlock::get_sub_Block: invalid scenario " +
   std::to_string( scenario )));
 return(v_Block[ scenario ]);
}

/*--------------------------------------------------------------------------*/

std::vector< ColVariable * > TwoStageStochasticBlock::get_first_stage_variables
 ( )
const {
 std::vector< ColVariable * > first_stage_vars;

 // Check if variables have been generated
 if( ! variables_generated( ) || v_Block.empty( ) ||
   v_paths_to_static_vars.empty( )) {
  return first_stage_vars; // Return empty vector
 }

 // Get the first scenario block (all scenarios have the same first-stage
 // variables due to non-anticipativity constraints)
 Block * scenario_0_block = v_Block[ 0 ];
 if( ! scenario_0_block ) {
  return first_stage_vars; // Return empty vector
 }

 // Iterate through all AbstractPaths to static (first-stage) variables
 for(const auto & path : v_paths_to_static_vars) {
  // Get the number of variables this path points to
  auto num_vars = path->get_number_elements< ColVariable >( scenario_0_block );

  // Get pointer to the first variable in the contiguous structure
  auto * first_var = path->get_element< ColVariable >( scenario_0_block );

  // Add all variables from this path to our result vector
  for(Index j = 0; j < num_vars; ++j) {
   first_stage_vars.push_back( first_var + j );
  }
 }

 return first_stage_vars;
}

/*--------------------------------------------------------------------------*/
/*----- METHODS DESCRIBING THE BEHAVIOR OF AN TwoStageStochasticBlock ------*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*------- METHODS FOR PRINTING & SAVING THE TwoStageStochasticBlock --------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::print( std::ostream & output , char vlvl ) const {
 output << std::endl << "TwoStageStochasticBlock with ";

 if( v_Block.empty( ))
  output << "no inner Block";
 else
  output << v_Block.size( ) << " sub-Block" << std::endl;
}

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::serialize( netCDF::NcGroup & group ) const {
 Block::serialize( group );

 // type
 group.putAtt( "type" , "TwoStageStochasticBlock" );

 // StochasticBlock
 auto sub_group = group.addGroup( "StochasticBlock" );
 get_sub_Block( 0 )->serialize( sub_group );

 // Serialize the DiscreteScenarioSet if present
 if( scenario_generator ) {
  // Check if it's a DiscreteScenarioSet (could support other types in future)
  auto * dss = dynamic_cast< DiscreteScenarioSet * >(scenario_generator);
  if( dss ) {
   auto scenario_group = group.addGroup( "DiscreteScenarioSet" );
   dss->serialize( scenario_group );
  }
  // Add support for other ScenarioGenerator types here if needed
 }
}

/*--------------------------------------------------------------------------*/
/*-------------------------- PRIVATE METHODS -------------------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::scale_scenario_objective(
  Block * scenario_block ,
  double weight ) {
 if( ! scenario_block ) return;

 Objective * obj = scenario_block->get_objective( );
 if( ! obj ) return;

 // Check if it's FRealObjective with LinearFunction
 auto * freal_obj = dynamic_cast< FRealObjective * >(obj);
 if( ! freal_obj ) {
  // For now, skip non-FRealObjective objectives
  // Could add warning or throw depending on requirements
  return;
 }

 auto * linear_func = dynamic_cast< LinearFunction * >(freal_obj->get_function
    ( ));
 if( ! linear_func ) {
  // Skip non-LinearFunction objectives
  return;
 }

 // Scale all variable coefficients
 Index n_vars = linear_func->get_num_active_var( );
 if( n_vars > 0 ) {
  Function::Vec_FunctionValue scaled_coeffs;
  scaled_coeffs.reserve( n_vars );

  for(Index i = 0; i < n_vars; ++i) {
   Function::FunctionValue coeff = linear_func->get_coefficient( i );
   scaled_coeffs.push_back( coeff * weight );
  }

  // Apply scaled coefficients using modify_coefficients
  linear_func->modify_coefficients(
    std::move( scaled_coeffs ) ,
    Range( 0 , n_vars ) ,
    eModBlck );
 }

 // Scale the constant term
 Function::FunctionValue constant = linear_func->get_constant_term( );
 linear_func->set_constant_term( constant * weight , eModBlck );
}

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::set_scenario_generator(
  ScenarioGenerator * generator ) {
 // Clean up previous generator if any
 if( scenario_generator ) delete scenario_generator;

 scenario_generator = generator;

 // Note: Scenarios are applied during block creation in deserialize().
 // This method is primarily for setting the generator before deserialization
 // or for future use cases where scenarios might need to be changed.
}

/*--------------------------------------------------------------------------*/
/*---------------- End File TwoStageStochasticBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
