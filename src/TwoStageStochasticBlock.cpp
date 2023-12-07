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

#include "AbstractPath.h"
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

/*--------------------------------------------------------------------------*/
/*--------- CONSTRUCTING AND DESTRUCTING TwoStageStochasticBlock -----------*/
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/*-------------------- Methods for handling Modification -------------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::add_Modification(
 sp_Mod mod , Observer::ChnlName chnl )
{
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
   return sub_Block->get_objective_sense();
 }
 catch( ... ) {}
 return Objective::eUndef;
}

/*--------------------------------------------------------------------------*/

StochasticBlock * TwoStageStochasticBlock::get_sub_Block
( Index stage , Index sub_block_index ) const {
 if( stage >= get_number_stages() )
  throw( std::invalid_argument( "TwoStageStochasticBlock::get_sub_Block: "
                                "invalid stage " + std::to_string( stage ) ) );
 if( sub_block_index >= num_sub_blocks_per_stage )
  throw( std::invalid_argument( "TwoStageStochasticBlock::get_sub_Block: "
                                "invalid sub-Block index " +
                                std::to_string( sub_block_index ) ) );
 const auto index = stage * num_sub_blocks_per_stage + sub_block_index;
 return static_cast< StochasticBlock * >( v_Block[ index ] );
}

/*--------------------------------------------------------------------------*/
/*----- METHODS DESCRIBING THE BEHAVIOR OF AN TwoStageStochasticBlock ------*/
/*--------------------------------------------------------------------------*/

double TwoStageStochasticBlock::get_future_cost( Index stage ,
                                                 Index sub_block_index ) const
{
 if( stage >= get_number_stages() )
  throw( std::invalid_argument(
   "TwoStageStochasticBlock::get_future_cost: invalid "
   "stage index: " + std::to_string( stage ) ) );
}

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::set_state(
 const Eigen::ArrayXd & values , Index stage ,
 Index sub_block_index )
{
 assert( stage < get_number_stages() );
 assert( sub_block_index < get_num_sub_blocks_per_stage() );
}

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::set_state(
 const std::vector< double > & values , Index stage ,
 Index sub_block_index )
{
 assert( stage < get_number_stages() );
 assert( sub_block_index < get_num_sub_blocks_per_stage() );
}

/*--------------------------------------------------------------------------*/

std::vector< double > TwoStageStochasticBlock::get_state(
 Index stage ,
 Index sub_block_index ) const
{
 assert( stage < get_number_stages() );
 assert( sub_block_index < get_num_sub_blocks_per_stage() );
}

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::set_admissible_state(
 Index stage , Index sub_block_index )
{
 assert( stage < get_number_stages() );
 assert( sub_block_index < get_num_sub_blocks_per_stage() );
}

/*--------------------------------------------------------------------------*/
/*------- METHODS FOR PRINTING & SAVING THE TwoStageStochasticBlock --------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::print( std::ostream & output , char vlvl ) const
{
 output << std::endl << "TwoStageStochasticBlock with ";

 if( v_Block.empty() )
  output << "no inner Block";
 else
  output << v_Block.size() << " sub-Blocks" << std::endl;
 }

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlock::serialize( netCDF::NcGroup & group ) const
{
 Block::serialize( group );

 // type

 group.putAtt( "type" , "TwoStageStochasticBlock" );

 // NumberStages

 const auto number_stages = get_number_stages();
 auto NumberStages_dim = group.addDim( "NumberStages" , number_stages );

 // NumSubBlocksPerStage

 group.addDim( "NumSubBlocksPerStage" , num_sub_blocks_per_stage );

 // StochasticBlock_i

 for( Index i = 0 ; i < number_stages ; ++i ) {
  auto sub_group = group.addGroup( "StochasticBlock_" + std::to_string( i ) );
  get_sub_Block( i )->serialize( sub_group );
 }

 // Scenarios

 scenario_set.serialize( group );

}

/*--------------------------------------------------------------------------*/
/*---------------- End File TwoStageStochasticBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
