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

#include "Block.h"
#include "Objective.h"
#include "ScenarioSet.h"
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
/// TwoStageStochasticBlock, representing a two-stage stochastic programming problem
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

 /// destructor
 virtual ~TwoStageStochasticBlock() {
  for( auto & block : v_Block )
   delete block;
  v_Block.clear();
 }

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

  // NumberStages

  Index number_stages = 2;
  ::SMSpp_di_unipi_it::deserialize_dim( group , "NumberStages" ,
                                        number_stages );

  // NumSubBlocksPerStage

  Index NumSubBlocksPerStage;
  if( ::SMSpp_di_unipi_it::deserialize_dim
      ( group , "NumSubBlocksPerStage" , NumSubBlocksPerStage ) ) {
   num_sub_blocks_per_stage = NumSubBlocksPerStage;
  }

  // StochasticBlocks

  v_Block.reserve( number_stages * num_sub_blocks_per_stage );

  for( Index i = 0 ; i < number_stages ; ++i )
   for( Index j = 0 ; j < num_sub_blocks_per_stage ; ++j )
    v_Block.push_back( deserialize_sub_Block( group , i ) );

  // Scenarios

  scenario_set.deserialize( group );

  Block::deserialize( group );
 }

/*--------------------------------------------------------------------------*/

 /// sets the number of sub-Blocks for each stage
 /** This function sets the number of sub-Blocks that must be constructed at
  * each stage. If this function is invoked after the sub-Blocks of this
  * TwoStageStochasticBlocks have been constructed, it has no effect. In
  * particular, it has no effect if it is invoked after deserialize() is
  * invoked.
  *
  * @param n The number of sub-Blocks that must be constructed at each stage.
  */
 void set_num_sub_blocks_per_stage( Index n ) {
  if( v_Block.empty() )
   num_sub_blocks_per_stage = n;
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

 /// returns the time horizon
 /** This function returns the time horizon associated with this SDDPBlock.
  *
  * @return The time horizon.
  */
 virtual Index get_time_horizon() const {
  return( v_Block.size() / num_sub_blocks_per_stage );
 }

/*--------------------------------------------------------------------------*/

 /// returns the number of stages
 /** This function returns the number of stages associated with this SDDPBlock.
  *
  * @return The number of stages.
  */
 virtual Index get_number_stages() const {
  return( v_Block.size() / num_sub_blocks_per_stage );
 }

/*--------------------------------------------------------------------------*/

 /// returns the number of sub-Blocks for each stage
 /** This function returns the number of sub-Blocks for each stage.
  *
  * @return The number of sub-Blocks for each stage.
  */
 Index get_num_sub_blocks_per_stage() const {
  return( num_sub_blocks_per_stage );
 }

/*--------------------------------------------------------------------------*/

 /// returns a sub-Block of this TwoStageStochasticBlock
 /** This function returns the sub-Block of index \p sub_block_index at the
  * given \p stage of this TwoStageStochasticBlock. The given \p stage must
  * be an integer between 0 and get_number_stages() - 1 and the index of the
  * sub-Block must be an integer between 0 and get_num_sub_blocks_per_stage()
  * 0 - 1. If any of them is an invalid index, an exception is thrown.
  *
  * @param stage The stage of the desired sub-Block. It must be a number
  *        between 0 and get_number_stages() - 1.
  *
  * @param sub_block_index The index of the desired sub-Block at the given \p
  *        stage. It must be a number between 0 and
  *        get_num_sub_blocks_per_stage() - 1.
  *
  * @return The sub-Block of this TwoStageStochasticBlock associated with the
  *         given \p stage and having index \p sub_block_index.
  */
 virtual StochasticBlock * get_sub_Block( Index stage ,
                                          Index sub_block_index = 0 ) const;

/*--------------------------------------------------------------------------*/

 /// returns the set of scenarios
 /** This function returns the set of scenarios. */
 const ScenarioSet & get_scenario_set() const {
  return( scenario_set );
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

 int get_objective_sense() const override;

/*--------------------------------------------------------------------------*/
/*-------------------- Methods for handling Modification -------------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for handling Modification
 *  @{ */

 void add_Modification( sp_Mod mod , ChnlName chnl = 0 ) override;

/*--------------------------------------------------------------------------*/
/*----- METHODS DESCRIBING THE BEHAVIOR OF AN TwoStageStochasticBlock ------*/
/*--------------------------------------------------------------------------*/
/** @name Methods describing the behavior of an TwoStageStochasticBlock
 * @{ */

 /// updates the sub-Block at the given stage for the given scenario
 /** This function updates the sub-Block whose index is \p sub_block_index at
  * the given \p stage for the given \p scenario.
  *
  * @param scenario_id The id of the scenario that must be set.
  *
  * @param sub_block_index The index of the sub-Block at the given \p
  *        stage. This must be an integer between 0 and
  *        get_num_sub_blocks_per_stage() - 1. */

 void set_scenario( Index scenario_id ,
                    Index stage ,
                    Index sub_block_index = 0 ) {
  auto sub_scenario_begin = scenario_set.sub_scenario_begin( scenario_id ,
                                                             stage );

  try {
   get_sub_Block( stage , sub_block_index )->set_data( sub_scenario_begin );
  }
  catch( const std::exception & e ) {
   std::cout << "TwoStageStochasticBlock::set_scenario: exception while "
                "setting scenario " << scenario_id << " of stage " << stage
             << ".\n" << e.what() << std::endl;
   std::exit( EXIT_FAILURE );
  }
 }

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

 /// Number of sub-Blocks for each stage
 Index num_sub_blocks_per_stage = 1;

 /// The set of scenarios
 ScenarioSet scenario_set;

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
 Block * deserialize_sub_Block( const netCDF::NcGroup & group , Index i ) {
  std::string sub_group_name = "StochasticBlock_" + std::to_string( i );
  auto sub_group = group.getGroup( sub_group_name );

  if( sub_group.isNull() ) {
   sub_group = group.getGroup( "StochasticBlock" );
   if( sub_group.isNull() )
    throw( std::logic_error( "TwoStageStochasticBlock::deserialize: neither "
                             "group '" + sub_group_name +
                             "' nor 'StochasticBlock' was found." ) );
   sub_group_name = "StochasticBlock";
  }

  auto type = sub_group.getAtt( "type" );
  if( type.isNull() )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: attribute "
                            "'type' of '" + sub_group_name +
                            "' must be present." ) );

  std::string type_name;
  type.getValues( type_name );

  if( type_name != "StochasticBlock" )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: attribute "
                            "'type' of '" + sub_group_name + "' must contain "
                            "'StochasticBlock'." ) );

  auto sub_Block = new_Block( sub_group , this );

  if( ! sub_Block )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: sub-group "
                            "'" + sub_group_name + "' is incomplete." ) );

  if( sub_group_name != "StochasticBlock" ) {

   // If StochasticBlock_i does not have subgroup "Block" then
   // "StochasticBlock" must have one.
   if( sub_group.getGroup( "Block" ).isNull() ) {

    auto StochasticBlock_group = group.getGroup( "StochasticBlock" );
    if( StochasticBlock_group.isNull() )
     throw( std::logic_error( "TwoStageStochasticBlock::deserialize: sub-group "
                              "'Block' was not provided neither in '" +
                              sub_group_name + "' nor in 'StochasticBlock'" ) );

    auto Block_group = StochasticBlock_group.getGroup( "Block" );
    if( Block_group.isNull() )
     throw( std::logic_error( "TwoStageStochasticBlock::deserialize: sub-group "
                              "'Block' was not provided neither in '" +
                              sub_group_name + "' nor in 'StochasticBlock'" ) );

    auto inner_block = new_Block( Block_group, this );
    if( ! inner_block )
     throw( std::logic_error( "TwoStageStochasticBlock::deserialize: the "
                              "'Block' sub-group of the 'StochasticBlock' group"
                              "has an invalid or incomplete description." ) );

    static_cast< StochasticBlock * >( sub_Block )->
     set_inner_block( inner_block );
   }

   // If StochasticBlock_i does not have the description of vector of
   // "DataMapping" then, if "StochasticBlock" has one, we use it.

   Index num_data_mappings;
   if( ! ::SMSpp_di_unipi_it::deserialize_dim( sub_group , "NumberDataMappings" ,
                                               num_data_mappings , true ) ) {

    auto StochasticBlock_group = group.getGroup( "StochasticBlock" );
    if( ! StochasticBlock_group.isNull() ) {

     if( ::SMSpp_di_unipi_it::deserialize_dim( StochasticBlock_group ,
                                               "NumberDataMappings" ,
                                               num_data_mappings , true ) ) {

      std::vector< std::unique_ptr< SimpleDataMappingBase > > data_mappings;
      data_mappings.reserve( num_data_mappings );
      SimpleDataMappingBase::deserialize
       ( group , data_mappings ,
         static_cast< StochasticBlock *>( sub_Block )->get_inner_block() );

      static_cast< StochasticBlock * >( sub_Block )->
       set_data_mappings( std::move( data_mappings ) );
     }
    }
   }
  }

  return( sub_Block );
 }

/*--------------------------------------------------------------------------*/

};   // end( class TwoStageStochasticBlock )

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

 }  // end( namespace SMSpp_di_unipi_it )

/*--------------------------------------------------------------------------*/

#endif  /* TwoStageStochasticBlock.h included */

/*--------------------------------------------------------------------------*/
/*----------------- End File TwoStageStochasticBlock.h ---------------------*/
/*--------------------------------------------------------------------------*/
