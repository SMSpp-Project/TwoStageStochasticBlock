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

void TwoStageStochasticBlock::generate_abstract_constraints( Configuration * stcc )
{
 if( v_paths_to_static_vars.empty() )
  return; // no Variable needs to be retrieved

 bool gen_seq_anchr_cnstrs = true;
 if( ( ! stcc ) && f_BlockConfig )
  stcc = f_BlockConfig->f_static_constraints_Configuration;
 if( auto sci = dynamic_cast< SimpleConfiguration< int > * >( stcc ) )
  gen_seq_anchr_cnstrs = sci->f_value;

 // Precompute inner_blocks for all scenarios
 std::vector< Block * > inner_blocks( get_number_scenarios() );
 for( int i = 0 ; i < get_number_scenarios() ; ++i ) {
  inner_blocks[ i ] = get_sub_Block( i )->get_inner_block();
  assert( inner_blocks[ i ] );
  // ensure here and now variables are generated
  inner_blocks[ i ]->generate_abstract_variables();
 }

 // Precompute variables for each scenario t, path i, and variable j
 boost::multi_array< std::vector< ColVariable * > , 2 > here_and_now_vars;
 here_and_now_vars.resize(
  boost::extents[ get_number_scenarios() ][ v_paths_to_static_vars.size() ] );
 for( int t = 0 ; t < get_number_scenarios() ; ++t ) {
  for( int i = 0 ; i < v_paths_to_static_vars.size() ; ++i ) {
   auto number_variables = v_paths_to_static_vars[ i ]->
    get_number_elements< ColVariable >( inner_blocks[ t ] );
   here_and_now_vars[ t ][ i ].resize( number_variables );
   // pointer to the first ColVariable of a contiguous structure,
   // i.e., a std::vector or boost::multi_array (since *static* vars)
   auto * elem = v_paths_to_static_vars[ i ]->
    get_element< ColVariable >( inner_blocks[ t ] );
   for( int j = 0 ; j < number_variables ; j++ )
    here_and_now_vars[ t ][ i ][ j ] = elem + j;
  }
 }

 here_and_now_const.resize(
  boost::extents[ get_number_scenarios() ][ v_paths_to_static_vars.size() ] );

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

} // end( TwoStageStochasticBlock::generate_abstract_constraints )

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

StochasticBlock * TwoStageStochasticBlock::get_sub_Block( Index scenario ) const {
 if( scenario >= get_number_scenarios() )
  throw( std::invalid_argument( "SDDPBlock::get_sub_Block: invalid scenario " +
                                std::to_string( scenario ) ) );
 return( static_cast< StochasticBlock * >( v_Block[ scenario ] ) );
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
/*---------------- End File TwoStageStochasticBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
