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

 delete v_Block.front();
 v_Block.clear();

 Constraint::clear( here_and_now_const );
}

/*--------------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::generate_abstract_constraints( Configuration * stcc )
{
 if( v_paths_to_vars.empty() )
  return; // no Variable needs to be retrieved

 bool gen_seq_anchr_cnstrs = true;
 if( ( ! stcc ) && f_BlockConfig )
  stcc = f_BlockConfig->f_static_constraints_Configuration;
 if( auto sci = dynamic_cast< SimpleConfiguration< int > * >( stcc ) )
  gen_seq_anchr_cnstrs = sci->f_value;

 LinearFunction::v_coeff_pair vars;

 if( gen_seq_anchr_cnstrs ) { // sequential constraints

  here_and_now_const.resize(
   boost::multi_array< FRowConstraint , 2 >::extent_gen()
   [ v_paths_to_vars.size() ][ get_number_scenarios() - 1 ] );

  // loop through each here-and-now variable
  for( int t = 0 ; t < v_paths_to_vars.size() ; ++t ) {

   // loop through each scenario, except the last one
   // scenario_0 = scenario_1
   // scenario_1 = scenario_2
   // . . .
   // scenario_i-1 = scenario_i
   for( int i = 0 ; i < get_number_scenarios() - 1 ; ++i ) {

    // create a constraint that ensures the t-th variable at the i-th scenario...
    auto inner_block = get_sub_Block( i )->get_inner_block();
    assert( inner_block );
    inner_block->generate_abstract_variables();

    vars.push_back( std::make_pair( v_paths_to_vars[ t ]->
                                    get_element< ColVariable >( inner_block ) ,
                                    1.0 ) );

    // ... is equal to the same t-th variable at (i+1)-th scenario
    inner_block = get_sub_Block( i + 1 )->get_inner_block();
    assert( inner_block );
    inner_block->generate_abstract_variables();

    vars.push_back( std::make_pair( v_paths_to_vars[ t ]->
                                    get_element< ColVariable >( inner_block ) ,
                                    -1.0 ) );

    here_and_now_const[ t ][ i ].set_both( 0.0 );
    here_and_now_const[ t ][ i ].set_function(
     new LinearFunction( std::move( vars ) ) );
   }
  }

 } else { // anchor constraints

  here_and_now_const.resize(
   boost::multi_array< FRowConstraint , 2 >::extent_gen()
   [ v_paths_to_vars.size() ][ get_number_scenarios() - 1 ] );

  // loop through each here-and-now variable
  for( int t = 0 ; t < v_paths_to_vars.size() ; ++t ) {

   // loop through each scenario, starting from the second one
   // scenario_0 = scenario_1
   // scenario_0 = scenario_2
   // . . .
   // scenario_0 = scenario_i
   for( int i = 1 ; i < get_number_scenarios() ; ++i ) {

    // create a constraint that ensures the t-th variable at the first scenario...
    auto inner_block = get_sub_Block( 0 )->get_inner_block();
    assert( inner_block );
    inner_block->generate_abstract_variables();

    vars.push_back( std::make_pair( v_paths_to_vars[ t ]->
                                    get_element< ColVariable >( inner_block ) ,
                                    1.0 ) );

    // ... is equal to the same t-th variable at i-th scenario
    inner_block = get_sub_Block( i )->get_inner_block();
    assert( inner_block );
    inner_block->generate_abstract_variables();

    vars.push_back( std::make_pair( v_paths_to_vars[ t ]->
                                    get_element< ColVariable >( inner_block ) ,
                                    -1.0 ) );

    here_and_now_const[ t - 1 ][ i ].set_both( 0.0 );
    here_and_now_const[ t - 1 ][ i ].set_function(
     new LinearFunction( std::move( vars ) ) );
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
