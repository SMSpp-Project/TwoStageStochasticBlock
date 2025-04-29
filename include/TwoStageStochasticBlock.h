/*--------------------------------------------------------------------------*/
/*------------------ File TwoStageStochasticBlock.h ------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Header file of TwoStageStochasticBlock, a class for representing a two-stage
 * stochastic programming problem.
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*----------------------------- DEFINITIONS --------------------------------*/
/*--------------------------------------------------------------------------*/

#ifndef __TwoStageStochasticBlock
 #define __TwoStageStochasticBlock
                      /* self-identification: #endif at the end of the file */

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include <ScenarioGenerator.h>

#include "Block.h"

#include "Objective.h"

#include "StochasticBlock.h"

/*--------------------------------------------------------------------------*/
/*----------------------------- NAMESPACE ----------------------------------*/
/*--------------------------------------------------------------------------*/

/// namespace for the Structured Modeling System++ (SMS++)
namespace SMSpp_di_unipi_it
{

/*--------------------------------------------------------------------------*/
/*------------------- CLASS TwoStageStochasticBlock ------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------- GENERAL NOTES --------------------------------*/
/*--------------------------------------------------------------------------*/
/// TwoStageStochasticBlock, representing a two-stage stochastic problem
/** The TwoStageStochasticBlock is a class that derives from Block and
 * represents a two-stage stochastic programming problem. */

class TwoStageStochasticBlock : public Block {

/*--------------------------------------------------------------------------*/
/*----------------------- PUBLIC PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

public:

/*--------------------------------------------------------------------------*/
/*--------- CONSTRUCTING AND DESTRUCTING TwoStageStochasticBlock -----------*/
/*--------------------------------------------------------------------------*/
/** @name Constructing and destructing TwoStageStochasticBlock
 *  @{ */

 /// constructor
 /** Constructs an TwoStageStochasticBlock with the given \p father Block.
  * The input parameter has a default value (\c nullptr), so that this can be
  * used as the void constructor.
  *
  * @param father A pointer to the father Block of this TwoStageStochasticBlock.
  */
 TwoStageStochasticBlock( Block * father = nullptr ) : Block( father ) { }

/*--------------------------------------------------------------------------*/

 /// destructor of TwoStageStochasticBlock

 virtual ~TwoStageStochasticBlock() override;

/*--------------------------------------------------------------------------*/
 /// generate the static constraint of the TwoStageStochasticBlock

 void generate_abstract_constraints( Configuration * stcc = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// loads TwoStageStochasticBlock out of an istream - not implemented yet

 void load( std::istream & input , char frmt = 0 ) override {
  throw( std::logic_error(
   "TwoStageStochasticBlock::load: method not implemented yet." ) );
  }

/*--------------------------------------------------------------------------*/
 /// de-serialize an TwoStageStochasticBlock out of netCDF::NcGroup
 /** The method takes a netCDF::NcGroup supposedly containing all the
  * information required to de-serialize the TwoStageStochasticBlock. Besides
  * the mandatory "type" attribute of any :Block, the group must contain the
  * following:
  *
  * -
  *
  * @param group A netCDF::NcGroup holding the data describing this
  *              TwoStageStochasticBlock.
  */

 void deserialize( const netCDF::NcGroup & group ) override {

  deserialize_dim( group , "NumberScenarios" , f_number_scenarios , false );

  // ScenarioGenerator

  /*auto scenario_group = group.getGroup( "ScenarioGenerator" );
  if( ! scenario_group.isNull() )
   scenario_gen = ScenarioGenerator::new_ScenarioGenerator( scenario_group );
  else
   throw( std::invalid_argument( "TwoStageStochasticBlock::deserialize: "
                                 "'ScenarioGenerator' group not found.") );

  scenario_gen->init_representative_pool( f_number_scenarios );*/

  // StochasticBlock

  v_Block.reserve( f_number_scenarios );

  for( Index i = 0 ; i < f_number_scenarios; ++i ) {

   auto * sb = deserialize_sub_Block( group );

   if( auto stochastic_block = dynamic_cast< StochasticBlock * >( sb ) ) {

    // Set the scenario for the current sub-Block
    // stochastic_block->set_scenario( scenario_gen->get_current_scenario() );

    // Scale the objective according to the current scenario probability
    if( auto inner_block = stochastic_block->get_inner_block() ) {
     // inner_block->scale( scenario_gen->get_current_scenario_probability() );
    }
   } else
    throw std::logic_error(
     "TwoStageStochasticBlock::deserialize: sub-Block is not a StochasticBlock." );

   // Add the sub-Block to the vector of blocks
   v_Block.push_back( sb );

   // Move to the next scenario
   /*if( ! scenario_gen->next_scenario() )
    throw( std::out_of_range( "TwoStageStochasticBlock::deserialize: "
                              "unable to move to the next scenario." ) );*/
  }

  // AbstractPath(s) to map both here-and-now static and dynamic variables

  auto static_path_group = group.getGroup( "StaticAbstractPath" );

  if( ! static_path_group.isNull() )
   AbstractPath::vector_deserialize( static_path_group , v_paths_to_static_vars );
  else
   throw( std::invalid_argument( "TwoStageStochasticBlock::deserialize: the "
                                 "group 'StaticAbstractPath' was not found." ) );

  auto dynamic_path_group = group.getGroup( "DynamicAbstractPath" );

  if( ! dynamic_path_group.isNull() )
   throw( std::invalid_argument( "TwoStageStochasticBlock::deserialize: "
                                 "cannot handle dynamic here-and-now variables "
                                 "right now." ) );
   // AbstractPath::vector_deserialize( dynamic_path_group , v_paths_to_dynamic_vars );

  Block::deserialize( group );
 }

/*--------------------------------------------------------------------------*/
/*-------- METHODS FOR Saving THE DATA OF THE TwoStageStochasticBlock ------*/
/*--------------------------------------------------------------------------*/
/** @name Saving the data of the TwoStageStochasticBlock
 *  @{ */

 void print( std::ostream & output , char vlvl = 0 ) const override;

/*--------------------------------------------------------------------------*/
 /// serialize an TwoStageStochasticBlock into a netCDF::NcGroup
 /** Serialize an TwoStageStochasticBlock into a netCDF::NcGroup with the format
  * explained in the comments of the deserialize() function.
  *
  * @param group The NcGroup in which this TwoStageStochasticBlock will be
  *              serialized. */

 void serialize( netCDF::NcGroup & group ) const override;

/*--------------------------------------------------------------------------*/
/*------ METHODS FOR READING THE DATA OF THE TwoStageStochasticBlock -------*/
/*--------------------------------------------------------------------------*/
/** @name Reading the data of the TwoStageStochasticBlock
    @{ */

 /// returns a sub-Block of this TwoStageStochasticBlock
 /** This function returns the sub-Block of index \p sub_block_index at the
  * given \p stage of this TwoStageStochasticBlock. The given \p stage must
  * be an integer between 0 and get_number_stages() - 1 and the index of the
  * sub-Block must be an integer between 0 and get_num_sub_blocks_per_stage()
  * 0 - 1. If any of them is an invalid index, an exception is thrown.
  *
  * @return The sub-Block of this TwoStageStochasticBlock associated with the
  *         given \p stage and having index \p sub_block_index.
  */
 virtual StochasticBlock * get_sub_Block( Index scenario ) const;

/*--------------------------------------------------------------------------*/

 /// returns the number of scenarios
 /** This function returns the number of scenarios. */
 Index get_number_scenarios( void ) const {
  return( f_number_scenarios );
 }

/*--------------------------------------------------------------------------*/

 /// returns the set of scenarios
 /** This function returns the set of scenarios. */
 const ScenarioGenerator * get_scenario_generator( void ) const {
  return( scenario_gen );
 }

/*--------------------------------------------------------------------------*/

 /// returns the sense of the Objective of the TwoStageStochasticBlock
 /** This function returns the sense of the Objective of the
  * TwoStageStochasticBlock, which is defined to be the sense of the
  * Objective of its first inner Block. If this TwoStageStochasticBlock has
  * no inner Block, this function returns Objective::eUndef.
  *
  * @return the sense of the Objective of the first inner Block of this
  *         TwoStageStochasticBlock if there is one. Otherwise, it returns
  *         Objective::eUndef. */

 int get_objective_sense( void ) const override;

/*--------------------------------------------------------------------------*/
/*-------------------- Methods for handling Modification -------------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for handling Modification
 *  @{ */

 void add_Modification( sp_Mod mod , ChnlName chnl = 0 ) override;

/*--------------------------------------------------------------------------*/
/*--------------------- PROTECTED PART OF THE CLASS ------------------------*/
/*--------------------------------------------------------------------------*/

protected:

/*--------------------------------------------------------------------------*/
/*-------------------------- PROTECTED METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*---------------------------- PROTECTED FIELDS  ---------------------------*/
/*--------------------------------------------------------------------------*/

/*---------------------------------- data ----------------------------------*/

 Index f_number_scenarios{};
 ///< The number of scenarios

 ScenarioGenerator * scenario_gen;
 ///< The scenario generator

 std::vector< std::unique_ptr< AbstractPath > > v_paths_to_static_vars;
 ///< The AbstractPath to the affected here-and-now static ColVariable

 std::vector< std::unique_ptr< AbstractPath > > v_paths_to_dynamic_vars;
 ///< The AbstractPath to the affected here-and-now static ColVariable

/*------------------------------- constraints ------------------------------*/

 ///< the here-and-now equality constraints
 boost::multi_array< std::vector< FRowConstraint > , 2 > here_and_now_const;

/*--------------------------------------------------------------------------*/
/*--------------------- PRIVATE PART OF THE CLASS --------------------------*/
/*--------------------------------------------------------------------------*/

private:

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE FIELDS ------------------------------*/
/*--------------------------------------------------------------------------*/

  SMSpp_insert_in_factory_h;

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/

 /// deserializes the i-th sub-Block out of the given group
 /** This auxiliary function deserializes the \p i-th sub-Block out of the
  * given \p group.
  *
  * @param group The netCDF::NcGroup containing the description of the
  *        sub-Block.
  *
  * @param i The index of the sub-Block to be deserialized.
  *
  * @return A pointer to the Block that was deserialized.
  */
 Block * deserialize_sub_Block( const netCDF::NcGroup & group ) {
  std::string sub_group_name = "StochasticBlock";
  auto StochasticBlock_group = group.getGroup( sub_group_name );

  if( StochasticBlock_group.isNull() )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: "
                            "'StochasticBlock' not found." ) );

  auto type = StochasticBlock_group.getAtt( "type" );
  if( type.isNull() )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: attribute "
                            "'type' of '" + sub_group_name +
                            "' must be present." ) );

  std::string type_name;
  type.getValues( type_name );
  if( type_name != "StochasticBlock" )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: attribute "
                            "'type' of '" + sub_group_name +
                            "' must contain 'StochasticBlock'." ) );

  auto StochasticBlock_block = new_Block( StochasticBlock_group , this );
  if( ! StochasticBlock_block )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: sub-group "
                            "'" + sub_group_name +
                            "' has an invalid or incomplete description." ) );

  auto Block_group = StochasticBlock_group.getGroup( "Block" );

  if( Block_group.isNull() )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: sub-group "
                            "'Block' was not provided in '" + sub_group_name ) );

  auto Block_block = new_Block( Block_group, this );
  if( ! Block_block )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: the "
                            "'Block' sub-group of the 'StochasticBlock' group"
                            "has an invalid or incomplete description." ) );

  static_cast< StochasticBlock * >( StochasticBlock_block )->
   set_inner_block( Block_block );

  /*Index num_data_mappings;
  if( ::SMSpp_di_unipi_it::deserialize_dim( StochasticBlock_group ,
                                            "NumberDataMappings" ,
                                            num_data_mappings , true ) ) {
   std::vector< std::unique_ptr< SimpleDataMappingBase > > data_mappings;
   data_mappings.reserve( num_data_mappings );
   SimpleDataMappingBase::deserialize
    ( group , data_mappings , static_cast< StochasticBlock * >(
     StochasticBlock_block )->get_inner_block() );

   static_cast< StochasticBlock * >( StochasticBlock_block )->
    set_data_mappings( std::move( data_mappings ) );
  }*/

  return( StochasticBlock_block );
 }

};   // end( class TwoStageStochasticBlock )

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

 }  // end( namespace SMSpp_di_unipi_it )

/*--------------------------------------------------------------------------*/

#endif  /* TwoStageStochasticBlock.h included */

/*--------------------------------------------------------------------------*/
/*----------------- End File TwoStageStochasticBlock.h ---------------------*/
/*--------------------------------------------------------------------------*/
