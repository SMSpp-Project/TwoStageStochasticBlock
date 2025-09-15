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
 * represents a two-stage stochastic programming problem.
 * 
 * This class builds the extensive form of a two-stage stochastic problem by:
 * - Creating N copies of the inner deterministic Block (one per scenario)
 * - Adding non-anticipativity constraints to ensure first-stage variables
 *   are the same across all scenarios
 * - Combining objectives from all scenarios (weighted by probabilities)
 * 
 * The class uses serialization/deserialization to create copies of the inner
 * Block, which works for all Block types without requiring them to implement
 * any special copy methods. The inner Block is serialized once and then
 * deserialized N times to create independent copies for each scenario.
 * 
 * Scenario data is applied through the apply_scenario_data() method, which
 * uses the DataMappings from the StochasticBlock to modify the parameters
 * of each scenario block. This design separates the structural creation of
 * the extensive form from the scenario data application, allowing flexible
 * integration with various scenario generation mechanisms.
 */

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
 /** Constructs a TwoStageStochasticBlock with the given \p father Block.
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
 /// generate the static variables of the TwoStageStochasticBlock
 /** This method generates the abstract variables by calling 
  * generate_abstract_variables() on all sub-blocks (scenario blocks). The 
  * TwoStageStochasticBlock itself doesn't create any variables - all 
  * variables exist within the scenario blocks.
  * 
  * @param stvv Configuration for variable generation (passed to sub-blocks)
  */
 
 void generate_abstract_variables( Configuration * stvv = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// generate the static constraint of the TwoStageStochasticBlock

 void generate_abstract_constraints( Configuration * stcc = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// generate_objective is not overridden - uses default behavior
 /** TwoStageStochasticBlock does not override generate_objective().
  * 
  * MILPSolver automatically aggregates objectives from all nested blocks
  * (scenario blocks) through its breadth-first traversal. This results in
  * an unweighted sum of all scenario objectives.
  * 
  * Users must manually divide the final objective value by the number of
  * scenarios to get the expected value (assuming uniform probabilities).
  * 
  * TODO: Once Objective::scale() is implemented in SMS++, this method could
  *       be overridden to apply probability weights to each scenario.
  */
 // void generate_objective( Configuration * objc = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// loads TwoStageStochasticBlock out of an istream - not implemented yet

 void load( std::istream & input , char frmt = 0 ) override {
  throw( std::logic_error(
   "TwoStageStochasticBlock::load: method not implemented yet." ) );
  }

/*--------------------------------------------------------------------------*/
 /// de-serialize a TwoStageStochasticBlock out of netCDF::NcGroup
 /** The method takes a netCDF::NcGroup supposedly containing all the
  * information required to de-serialize the TwoStageStochasticBlock. Besides
  * the mandatory "type" attribute of any :Block, the group must contain the
  * following:
  *
  * - NumberScenarios: dimension specifying the number of scenarios
  * - StochasticBlock: group containing the StochasticBlock definition
  * - StaticAbstractPath: group with AbstractPaths to first-stage variables
  * - DynamicAbstractPath (optional): currently not supported
  *
  * This method creates N copies of the inner block (one per scenario) by
  * deserializing directly from the Block group in the netCDF data. This
  * avoids the need for temporary files and creates independent copies
  * efficiently.
  *
  * After deserialization, scenario-specific data should be applied to each
  * block using the apply_scenario_data() method.
  *
  * @param group A netCDF::NcGroup holding the data describing this
  *              TwoStageStochasticBlock.
  */

 void deserialize( const netCDF::NcGroup & group ) override {

  deserialize_dim( group , "NumberScenarios" , f_number_scenarios , false );

  // Get the StochasticBlock group which contains the inner Block definition
  auto StochasticBlock_group = group.getGroup( "StochasticBlock" );
  if( StochasticBlock_group.isNull() )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: "
                            "'StochasticBlock' not found." ) );

  // Get the Block group that contains the inner block definition
  auto Block_group = StochasticBlock_group.getGroup( "Block" );
  if( Block_group.isNull() )
   throw( std::logic_error( "TwoStageStochasticBlock::deserialize: "
                            "'Block' sub-group not found in 'StochasticBlock'." ) );

  // Deserialize the StochasticBlock (with DataMappings)
  auto * sb = deserialize_sub_Block( group );
  stochastic_block = dynamic_cast< StochasticBlock * >( sb );
  
  if( ! stochastic_block )
   throw std::logic_error(
    "TwoStageStochasticBlock::deserialize: sub-Block is not a StochasticBlock." );
  
  // Get the inner block which will be used as the source for verification
  auto * inner_block_source = stochastic_block->get_inner_block();
  if( ! inner_block_source )
   throw std::logic_error(
    "TwoStageStochasticBlock::deserialize: StochasticBlock has no inner block." );

  v_Block.reserve( f_number_scenarios );

  // Create N copies of the inner block by deserializing directly from Block_group
  for( Index i = 0 ; i < f_number_scenarios; ++i ) {
   
   // Create a copy of the inner block through deserialization
   Block * block_copy = Block::new_Block( Block_group , this );
   
   if( ! block_copy )
    throw std::logic_error(
     "TwoStageStochasticBlock::deserialize: failed to create block copy "
     "through deserialization for scenario " + std::to_string(i) );
   
   // Note: Scenario-specific data should be applied after deserialization
   // using the apply_scenario_data() method. This allows external control
   // over what scenario data is applied to each block copy.
   
   v_Block.push_back( block_copy );
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
 /// serialize a TwoStageStochasticBlock into a netCDF::NcGroup
 /** Serialize a TwoStageStochasticBlock into a netCDF::NcGroup with the format
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
 /** This function returns the sub-Block at the given scenario index.
  * The scenario index must be between 0 and get_number_scenarios() - 1.
  * If the index is invalid, an exception is thrown.
  *
  * @param scenario The index of the scenario (0 to n_scenarios-1)
  * @return The inner Block copy for the specified scenario
  */
 virtual Block * get_sub_Block( Index scenario ) const;

/*--------------------------------------------------------------------------*/

 /// returns the number of scenarios
 /** This function returns the number of scenarios. */
 Index get_number_scenarios( void ) const {
  return( f_number_scenarios );
 }

/*--------------------------------------------------------------------------*/

 /// applies scenario data to a specific scenario block
 /** This method applies scenario-specific data to one of the scenario blocks
  * using DataMappings. It creates a StochasticBlock wrapper for each scenario
  * block on first use, with DataMappings recreated to target the specific
  * block copy.
  * 
  * The implementation:
  * - Creates StochasticBlock wrappers lazily when first needed
  * - Recreates DataMappings from the original StochasticBlock template
  * - Applies scenario data through the wrapper's set_data() method
  * 
  * @param scenario_index Index of the scenario (0 to n_scenarios-1)
  * @param scenario_data Vector containing the scenario realization data
  * @param issuePMod Parameter for physical modifications
  * @param issueAMod Parameter for abstract modifications
  * 
  * @throw std::invalid_argument If scenario_index is out of range
  * @throw std::logic_error If no StochasticBlock template is available
  */
 void apply_scenario_data( Index scenario_index, 
                          const std::vector<double>& scenario_data,
                          c_ModParam issuePMod = eNoBlck,
                          c_ModParam issueAMod = eNoBlck );

/*--------------------------------------------------------------------------*/

 /// returns the first-stage (here-and-now) variables
 /** This function returns all first-stage variables from the first scenario.
  * These are the variables that must have the same value across all scenarios
  * due to non-anticipativity constraints. The method requires that abstract
  * variables have been generated first (by calling generate_abstract_variables).
  * 
  * @return A vector containing pointers to all first-stage ColVariable objects
  *         from scenario 0. Returns empty vector if variables haven't been
  *         generated yet or if there are no first-stage variables.
  * 
  * @note The returned variables are from scenario 0, but due to non-anticipativity
  *       constraints, they represent the same decisions across all scenarios.
  */
 std::vector< ColVariable * > get_first_stage_variables( void ) const;

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

 /// states that the Variable have been generated
 void set_variables_generated( void ) { AR |= HasVar; }

 /// states that the Constraint have been generated
 void set_constraints_generated( void ) { AR |= HasCst; }

 /// states that the Objective has been generated
 void set_objective_generated( void ) { AR |= HasObj; }

 /// indicates whether the Variable have been generated
 bool variables_generated( void ) const { return( AR & HasVar ); }

 /// indicates whether the Constraint have been generated
 bool constraints_generated( void ) const { return( AR & HasCst ); }

 /// indicates whether the Objective has been generated
 bool objective_generated( void ) const { return( AR & HasObj ); }

/*--------------------------------------------------------------------------*/
/*---------------------------- PROTECTED FIELDS  ---------------------------*/
/*--------------------------------------------------------------------------*/

/*---------------------------------- data ----------------------------------*/

 Index f_number_scenarios{};
 ///< The number of scenarios
 
 StochasticBlock * stochastic_block = nullptr;
 ///< The StochasticBlock containing DataMappings for applying scenario data

 std::vector< std::unique_ptr< AbstractPath > > v_paths_to_static_vars;
 ///< The AbstractPath to the affected here-and-now static ColVariable

 std::vector< std::unique_ptr< AbstractPath > > v_paths_to_dynamic_vars;
 ///< The AbstractPath to the affected here-and-now dynamic ColVariable

/*------------------------------- constraints ------------------------------*/

 ///< the here-and-now equality constraints
 boost::multi_array< std::vector< FRowConstraint > , 2 > here_and_now_const;

/*--------------------------------------------------------------------------*/
/*--------------------- PRIVATE PART OF THE CLASS --------------------------*/
/*--------------------------------------------------------------------------*/

private:

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/

 /// apply scenario to a specific Block using StochasticBlock's DataMappings
 /** This private helper method applies scenario data to a specific Block by 
  * retrieving the DataMappings from the StochasticBlock and using them to
  * update the target Block's parameters. This is used internally to apply
  * scenarios to the Block copies created for each scenario.
  *
  * @param target_block The Block to which the scenario data will be applied.
  * @param scenario_data The scenario data to be applied.
  * @param issuePMod Indicates if and how a "physical" Modification should 
  * be issued.
  * @param issueAMod Indicates if and how an "abstract" Modification should 
  * be issued.
  */
 void apply_scenario_to_block( Block * target_block,
                               const std::vector< double > & scenario_data,
                               c_ModParam issuePMod = eNoBlck,
                               c_ModParam issueAMod = eNoBlck );

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE FIELDS ------------------------------*/
/*--------------------------------------------------------------------------*/

 ///< bit-wise coded: what abstract is there
 unsigned char AR{};

 static constexpr unsigned char HasVar = 1;
 ///< first bit of AR == 1 if the Variables have been constructed

 static constexpr unsigned char HasCst = 2;
 ///< second bit of AR == 1 if the Constraints have been constructed

 static constexpr unsigned char HasObj = 4;
 ///< third bit of AR == 1 if the Objective has been constructed

  SMSpp_insert_in_factory_h;

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE METHODS -----------------------------*/
/*--------------------------------------------------------------------------*/

 /// deserializes a StochasticBlock from the given group
 /** This auxiliary function deserializes a StochasticBlock from the given
  * netCDF group, including its inner Block and DataMappings. The DataMapping
  * deserialization is properly enabled, allowing the StochasticBlock to
  * modify its inner Block's parameters based on scenario data.
  *
  * @param group The netCDF::NcGroup containing the TwoStageStochasticBlock
  *              description, which includes the StochasticBlock subgroup.
  *
  * @return A pointer to the deserialized StochasticBlock.
  * 
  * @throw std::logic_error If required groups are missing or have invalid types.
  * 
  * @note The returned StochasticBlock contains the inner Block that serves as
  *       the template for creating scenario-specific copies. The StochasticBlock
  *       itself stores the DataMappings that define which parameters vary
  *       across scenarios.
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

  Index num_data_mappings;
  if( deserialize_dim( StochasticBlock_group , "NumberDataMappings" ,
                       num_data_mappings , true ) ) {
   std::vector< std::unique_ptr< SimpleDataMappingBase > > data_mappings;
   data_mappings.reserve( num_data_mappings );
   SimpleDataMappingBase::deserialize
    ( StochasticBlock_group , data_mappings , static_cast< StochasticBlock * >(
     StochasticBlock_block )->get_inner_block() );

   static_cast< StochasticBlock * >( StochasticBlock_block )->
    set_data_mappings( std::move( data_mappings ) );
  }

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
