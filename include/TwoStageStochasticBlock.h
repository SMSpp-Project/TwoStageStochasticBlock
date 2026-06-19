/*--------------------------------------------------------------------------*/
/*------------------ File TwoStageStochasticBlock.h ------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Header file of TwoStageStochasticBlock, a class for representing a two-stage
 * stochastic programming problem.
 *
 * \author Antonio Frangioni \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Antonio Frangioni and Donato Meoli
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

#include "StochasticBlock.h"

#include "ScenarioGenerator.h"

#include "DiscreteScenarioSet.h"

/*--------------------------------------------------------------------------*/
/*----------------------------- NAMESPACE ----------------------------------*/
/*--------------------------------------------------------------------------*/

/// namespace for the Structured Modeling System++ (SMS++)
namespace SMSpp_di_unipi_it {

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
 *   are the same across all scenarios,
 * - Combining objectives weighted by probabilities.
 *
 *   TODO: We need Objective::scale() to be implemented. Currently we have a
 * workaround for LinearFunction objectives (like
 * CapacitatedFacilityLocationBlock or UCBlock) that should be replaced.
 *
 * The class uses serialization/deserialization to create copies of the inner
 * Block, which works for all Block types without requiring them to implement
 * any special copy methods. The inner Block is serialized once and then
 * deserialized N times to create independent copies for each scenario.
 *
 * Scenario data is applied automatically when a ScenarioGenerator is provided,
 * either through deserialization (when a ScenarioSet is present in the netCDF)
 * or by explicitly setting one via set_scenario_generator(). The class uses
 * the StochasticBlock as a temporary "applicator" for scenarios: it sets each
 * block copy as the inner block of StochasticBlock, applies the scenario data
 * through StochasticBlock's set_data() method (which uses the DataMappings),
 * then restores the original inner block. This approach eliminates the need
 * for DataMapping::set_caller() and provides a cleaner architecture.
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
  * @param father A pointer to the father Block of this
  *               TwoStageStochasticBlock.
  */
 explicit TwoStageStochasticBlock( Block * father = nullptr )
  : Block( father ) {}

/*--------------------------------------------------------------------------*/

 /// constructor with ScenarioGenerator
 /** Constructs a TwoStageStochasticBlock with the given \p father Block and
  * \p generator ScenarioGenerator. When a generator is provided, scenarios
  * will be automatically applied to block copies during creation.
  * The TwoStageStochasticBlock takes ownership of the generator and will
  * delete it in the destructor.
  *
  * @param father A pointer to the father Block of this
  *               TwoStageStochasticBlock.
  *
  * @param generator A pointer to the ScenarioGenerator for automatic scenario
  *                  application.
  */
 TwoStageStochasticBlock( Block * father , ScenarioGenerator * generator )
  : Block( father ) , scenario_generator( generator ) {}

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
 /// generate the objective of the TwoStageStochasticBlock
 /** Generates the objective function for the two-stage stochastic block.
  *
  * This method first generates objectives for all scenario blocks, then
  * scales each scenario's objective by its probability weight (if a
  * ScenarioGenerator is available). Finally, it calls the base class
  * method to aggregate the weighted objectives.
  *
  * If no ScenarioGenerator is available, scenarios are implicitly given
  * equal weights (unscaled objectives are summed).
  *
  * @param objc optional Configuration for objective generation
  */
 void generate_objective( Configuration * objc = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// loads TwoStageStochasticBlock out of an istream - not implemented yet

 void load( std::istream & input , char frmt = 0 ) override {
  throw(std::logic_error(
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
  * If a ScenarioGenerator group is present (e.g., DiscreteScenarioSet), it
  * will be automatically deserialized and used to apply scenario data to each
  * block copy.
  *
  * @param group A netCDF::NcGroup holding the data describing this
  *              TwoStageStochasticBlock.
  */

 void deserialize( const netCDF::NcGroup & group ) override {
  deserialize_dim( group , "NumberScenarios" , f_number_scenarios , false );

  // Get the StochasticBlock group which contains the inner Block definition
  auto StochasticBlock_group = group.getGroup( "StochasticBlock" );
  if( StochasticBlock_group.isNull() )
   throw(std::logic_error( "TwoStageStochasticBlock::deserialize: "
    "'StochasticBlock' not found." ) );

  // Get the Block group that contains the inner block definition
  auto Block_group = StochasticBlock_group.getGroup( "Block" );
  if( Block_group.isNull() )
   throw(std::logic_error( "TwoStageStochasticBlock::deserialize: "
    "'Block' sub-group not found in 'StochasticBlock'." ) );

  // Deserialize the StochasticBlock (with DataMappings)
  // The new_Block function will handle all the deserialization including
  // inner block and DataMappings
  auto * sb = new_Block( StochasticBlock_group , this );
  stochastic_block = dynamic_cast< StochasticBlock * >(sb);

  if( ! stochastic_block )
   throw std::logic_error( "TwoStageStochasticBlock::deserialize: sub-Block is "
    "not a StochasticBlock." );

  // Save the original inner block to restore later
  auto * original_inner_block = stochastic_block->get_inner_block();
  if( ! original_inner_block )
   throw std::logic_error( "TwoStageStochasticBlock::deserialize: "
    "StochasticBlock has no inner block." );

  v_Block.reserve( f_number_scenarios );

  // Check for a ScenarioGenerator group (could be DiscreteScenarioSet or
  // other)
  auto DiscreteScenarioSet_group = group.getGroup( "DiscreteScenarioSet" );
  bool has_discrete_scenarios = ! DiscreteScenarioSet_group.isNull();

  if( has_discrete_scenarios ) {
   // Create and deserialize the DiscreteScenarioSet
   auto * dss = new DiscreteScenarioSet();
   dss->deserialize( DiscreteScenarioSet_group );
   scenario_generator = dss; // Always owned

   // Initialize the scenario generator
   scenario_generator->init_representative_pool( f_number_scenarios );
  }

  // Create blocks with scenarios applied using StochasticBlock as applicator
  for( Index i = 0; i < f_number_scenarios ; ++i ) {
   // Create a fresh copy of the inner block through deserialization
   Block * block_copy = new_Block( Block_group , this );

   if( ! block_copy )
    throw std::logic_error(
     "TwoStageStochasticBlock::deserialize: failed to create block copy "
     "through deserialization for scenario " +
     std::to_string( i ) );

   if( has_discrete_scenarios ) {
    // Apply scenario data if DiscreteScenarioSet is available
    // 1. Set the copy as inner block of StochasticBlock (don't destroy previous)
    stochastic_block->set_inner_block( block_copy , false );

    // 2. Update all DataMapping callers to point to the new block
    const auto & data_mappings = stochastic_block->get_data_mappings();
    for( auto & dm : data_mappings )
     dm->set_caller_from_reference( block_copy );

    // 3. Apply the current scenario through StochasticBlock
    auto scenario_data = scenario_generator->get_current_scenario();
    // Convert span to vector for compatibility with set_data
    std::vector scenario_vec( scenario_data.begin() , scenario_data.end() );
    stochastic_block->set_data( scenario_vec );

    // Move to next scenario for next iteration
    if( i < f_number_scenarios - 1 )
     scenario_generator->next_scenario();
   }

   // Add the block to v_Block
   v_Block.push_back( block_copy );
  }

  // Restore the original inner block
  stochastic_block->set_inner_block( original_inner_block , false );

  // AbstractPath(s) to map both here-and-now static and dynamic variables

  auto static_path_group = group.getGroup( "StaticAbstractPath" );

  if( ! static_path_group.isNull() )
   AbstractPath::vector_deserialize( static_path_group , v_paths_to_static_vars );
  else
   throw(std::invalid_argument( "TwoStageStochasticBlock::deserialize: the "
    "group 'StaticAbstractPath' was not found." ) );

  auto dynamic_path_group = group.getGroup( "DynamicAbstractPath" );

  if( ! dynamic_path_group.isNull() )
   throw(std::invalid_argument( "TwoStageStochasticBlock::deserialize: "
    "cannot handle dynamic here-and-now variables "
    "right now." ) );
  // AbstractPath::vector_deserialize( dynamic_path_group ,
  // v_paths_to_dynamic_vars );

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
  *              serialized.
  */

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
  *
  * @return The inner Block copy for the specified scenario
  */
 virtual Block *get_sub_Block( Index scenario ) const;

/*--------------------------------------------------------------------------*/

 /// returns the Block against which the here-and-now paths are resolved
 /** Returns the Block relative to which the here-and-now AbstractPath in
  * v_paths_to_static_vars are resolved for the given \p scenario. For a plain
  * TwoStageStochasticBlock this is the scenario sub-Block itself, i.e.,
  * get_sub_Block( scenario ). Derived classes that nest a further stochastic
  * structure (such as MultiStageStochasticBlock, whose sub-Blocks are
  * themselves TwoStageStochasticBlock) override this to descend into the
  * representative sub-Block where the here-and-now variables physically live,
  * so that the inherited constraint, Variable and Solution machinery keeps
  * working unchanged. */

 virtual Block * get_first_stage_block( Index scenario ) const {
  return( get_sub_Block( scenario ) );
  }

/*--------------------------------------------------------------------------*/

 /// returns the number of scenarios
 /** This function returns the number of scenarios. */
 Index get_number_scenarios( void ) const { return( f_number_scenarios ); }

/*--------------------------------------------------------------------------*/

 /// sets the ScenarioGenerator for automatic scenario application
 /** This method sets a ScenarioGenerator that will be used to automatically
  * apply scenario data to block copies. The generator must provide scenarios
  * compatible with the StochasticBlock's DataMappings.
  * The TwoStageStochasticBlock takes ownership of the generator and will
  * delete it in the destructor.
  *
  * @param generator Pointer to the ScenarioGenerator
  */
 void set_scenario_generator( ScenarioGenerator * generator );

/*--------------------------------------------------------------------------*/

 /// returns the first-stage (here-and-now) variables
 /** This function returns all first-stage variables from the first scenario.
  * These are the variables that must have the same value across all scenarios
  * due to non-anticipativity constraints. The method requires that abstract
  * variables have been generated first (by calling
  * generate_abstract_variables).
  *
  * @return A vector containing pointers to all first-stage ColVariable objects
  *         from scenario 0. Returns empty vector if variables haven't been
  *         generated yet or if there are no first-stage variables.
  *
  * @note The returned variables are from scenario 0, but due to
  * non-anticipativity constraints, they represent the same decisions across
  * all scenarios.
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
 /// return the paths to static here-and-now variables
 /** Returns a const reference to the vector of AbstractPaths that point to
  * the static (first-stage, here-and-now) variables in the scenario blocks.
  *
  * @return const reference to v_paths_to_static_vars
  */

 const std::vector< std::unique_ptr< AbstractPath > > &
                       get_paths_to_static_here_and_now_vars( void ) const {
  return( v_paths_to_static_vars );
  }

/*--------------------------------------------------------------------------*/
 /// return the paths to dynamic here-and-now variables
 /** Returns a const reference to the vector of AbstractPaths that point to
  * the dynamic (first-stage, here-and-now) variables in the scenario blocks.
  *
  * @return const reference to v_paths_to_dynamic_vars
  */

 const std::vector< std::unique_ptr< AbstractPath > > &
                       get_paths_to_dynamic_here_and_now_vars( void ) const {
  return( v_paths_to_dynamic_vars );
  }

/*--------------------------------------------------------------------------*/
/*----------------------- Methods for handling Solution --------------------*/
/*--------------------------------------------------------------------------*/
 /// returns a TwoStageStochasticBlockSolution with the current solution
 /** Returns a TwoStageStochasticBlockSolution representing the current
  * solution status of this TwoStageStochasticBlock. What kind of solution
  * is saved depends on the integer value ws, obtained as follows:
  *
  * - if solc != nullptr and it is a SimpleConfiguration< int >, then
  *   ws == solc->f_value:
  *
  * - if solc == nullptr, f_BlockConfig != nullptr,
  *   f_BlockConfig->f_solution_Configuration != nullptr and it
  *   is a SimpleConfiguration< int >, ws is its f_value
  *
  * - if solc == nullptr, f_BlockConfig != nullptr,
  *   f_BlockConfig->f_solution_Configuration != nullptr and it
  *   is a SimpleConfiguration< std::pair< int , Configuration * >, ws is
  *   its f_value->first
  *
  * - otherwise ws is 3 (save here-and-now variables and and the individual
  *   scenarios Solution)
  *
  * The encoding of ws is bit-wise:
  *
  *   = bit 0 (& 1): means "save the here-and-now variables"
  *
  *   = bit 1 (& 2): means "save the individual scenarios Solution"
  *
  *   = bit 2 (& 4): means "save the dual variables of the non-anticipativity
  *                  constraints"
  *
  * If a SimpleConfiguration< int > is used, then all the subsequent bits
  * are passed as a the int value in a SimpleConfiguration< int > that is
  * passed as Configuration to get_Solution() when it is called to read() the
  * Solution of the individual scenario; if such values are 0, no
  * Configuration is passed (the default is used). If a
  * SimpleConfiguration< std::pair< int , Configuration * > is used, then its
  * f_value->second is passed to the inner get_Solution() instead.
  *
  * Note that, although the method clearly returns a
  * TwoStageStochasticBlockSolution, formally the return type is Solution *.
  * This is because it is not possible to forward declare
  * TwoStageStochasticBlockSolution as a derived class from Solution, nor to
  * define TwoStageStochasticBlockSolution before TwoStageStochasticBlock
  * because the former uses some type information declared in the latter. */

 Solution * get_Solution( Configuration * solc = nullptr ,
                          bool emptys = false ) override;

/** @} ---------------------------------------------------------------------*/
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

 /// helper method to scale a single scenario's objective by a weight
 /** Recursively scales the objective functions of \p block and of all its
  * nested Blocks by the given \p weight, supporting LinearFunction and
  * DQuadFunction objectives. It is protected so that derived classes (such as
  * MultiStageStochasticBlock) can reuse it to apply their own per-sub-Block
  * weights. */
 void scale_objective_recursive( Block * block , double weight );

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

 ScenarioGenerator * scenario_generator = nullptr;
 ///< The ScenarioGenerator for automatic scenario application (owned)

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

//  /// apply scenarios to all blocks using the ScenarioGenerator
//  /** This private helper method applies scenario data to all block copies
//   * using the configured ScenarioGenerator. It is called during deserialization
//   * or when blocks are created if a generator is available.
//   *
//   * @param issuePMod Indicates if and how a "physical" Modification should
//   * be issued.
//   * @param issueAMod Indicates if and how an "abstract" Modification should
//   * be issued.
//   *
//   * @throw std::logic_error If no ScenarioGenerator is configured
//   */
//  void apply_scenarios_from_generator(
//    c_ModParam issuePMod = eNoBlck ,
//    c_ModParam issueAMod = eNoBlck );

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

 }; // end( class TwoStageStochasticBlock )

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*---------------- CLASS TwoStageStochasticBlockSolution -------------------*/
/*--------------------------------------------------------------------------*/
/** The TwoStageStochasticBlockSolution class derives from Solution and
 * implements it for TwoStageStochasticBlock. As such it may contain any of:
 *
 * - one single copy of the values of the here_and_now variables;
 *
 * - the complete Solution for all the Block corresponding to each individual
 *   scenario;
 *
 * - the dual variables of the non-anticipativity constraints. */

class TwoStageStochasticBlockSolution : public Solution
{

/*--------------------------------------------------------------------------*/
/*----------------------- PUBLIC PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 public:

/*------------------------------- FRIENDS ----------------------------------*/

 friend TwoStageStochasticBlock;  ///< make TwoStageStochasticBlock friend

/*------ CONSTRUCTING AND DESTRUCTING TwoStageStochasticBlockSolution ------*/

 /// constructor

 explicit TwoStageStochasticBlockSolution( void ) : Solution() ,
  f_inner_Config( nullptr ) {}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// deserialize a TwoStageStochasticBlockSolution from a netCDF::NcGroup

 void deserialize( const netCDF::NcGroup & group ) override;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// destructor

 ~TwoStageStochasticBlockSolution() override {
  for( auto si : v_scenario_solutions )
   delete( si );
  }

/*- METHODS DESCRIBING THE BEHAVIOR OF A TwoStageStochasticBlockSolution --*/

 void read( const Block * block ) override final;

 void write( Block * block ) override final;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
 /// serialize a TwoStageStochasticBlockSolution into a netCDF::NcGroup
 /** Serialize a TwoStageStochasticBlockSolution into a netCDF::NcGroup. The
  * format is the following:
  *
  * - The dimension "TotalNumberHereAndNow" containing the total number of
  *   here_and_now variables. The dimension is mandatory if either
  *   "HereAndNow" or "DualValues" exist, and optional otherwise.
  *
  * - The variable "HereAndNow", of type netCDF::NcDouble and indexed over
  *   "TotalNumberHereAndNow". It contains the values of the here_and_now
  *   variables, in row-major format (first all the variables of the first
  *   group, then all the variables of the second group, ...), with
  *   "HereAndNowStart" [see below] used to tell where each group starts
  *   (and therefore ends). The variable is optional, but if it exists then
  *   also the dimensios "NumberHereAndNow" and "TotalNumberHereAndNow" must
  *   exist.
  *
  * - The dimension "NumberHereAndNowGroups" containing the number of groups
  *   of here_and_now variables. The dimension is optional, if it is not
  *   there and "HereAndNow" esists it is taken to be 1 (only one group).
  *
  * - The variable "HereAndNowStart", of type netCDF::NcInt and indexed over
  *   "NumberHereAndNow". The values of the i-th group are saved into the
  *   elements of HereAndNow[] with indices in the closed-open interval
  *   [ HereAndNowStart[ i ] , HereAndNowStart[ i + 1 ] ), except for the
  *   last group for which HereAndNowStart[ i + 1 ] is undefined and whose
  *   elements are saved into the elements of HereAndNow[] with indices in
  *   the interval [ HereAndNowStart[ i ] , TotalNumberHereAndNow ). The
  *   variable is optional, but it must exist if "HereAndNow" exists.
  *
  * - The dimension "NumberScenarios" containing the number of scenarios. The
  *   dimension is mandatory if either "ScenarioSolution_0" of
  *   "ScenarioSolutionPrefix" (see below) are there and optional otherwise.
  *
  * - If "NumberScenarios" is defined, optional Solution data for the
  *   sub-Block corresponding to each scenarios, under two possible
  *   alternative forms:
  *
  *   = either the optional string "ScenarioSolutionPrefix", meaning that
  *     the Solution for each scenario object for the sub-Block
  *     corresponding to scenario T = 0, ..., NumberScenarios - 1, is saved
  *     into the file with name "ScenarioSolutionPrefix_T.nc4"; these files
  *     must then exist;
  *
  *   = or the groups "ScenarioSolution_T" for T = 0, ..., NumberScenarios
  *     - 1, each containing the Solution object for the sub-Block
  *     corresponding to scenario T. The groups are optional but either they
  *     are all there or none is, hence one can just check the esistence of
  *     Solution_0: if it exists then all other ones, and the dimension
  *     "NumberScenarios", must exist.
  *
  *   During deserialize(), the first form is checked first, if it does not
  *   exist the second one is checked, if none exist no Solution data is
  *   present. During serialize(), the format is chosen by the current value
  *   of the f_scenario_solution_fprefix field (see set_inner_Config()): if
  *   it is empty() then the second (group-based) format is used, otherwise
  *   the first (file-based) is.
  *
  * - The variable "DualValues",  of type netCDF::NcDouble indiced over both
  *   the dimensions "TotalNumberHereAndNow" and "NumberScenarios", such that
  *   DualValues[ i ][ s ] contains the optimal value of the dual variable
  *   corresponding to the non-anticipaticity constraint regarding the
  *   here_and_now variable i (with the same ordering as that of "HereAndNow")
  *   for scenario s. There are two forms of these constraints, so the actual
  *   values will depend on which form was chosen when the
  *   TwoStageStochasticBlock was built, but in alla cases notice that there
  *   are NumberScenarios - 1 non-anticipaticity constraints regarding each
  *   variable. The variable is optional. */

 void serialize( netCDF::NcGroup & group ) const override;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

 TwoStageStochasticBlockSolution * scale( double factor ) const override;

 void sum( const Solution * solution , double multiplier ) override;

 TwoStageStochasticBlockSolution * clone( bool empty = false ) const override;

/*--------------------------------------------------------------------------*/
 /// set the inner Config
 /** Sets the f_inner_Config field, which is used in read() to Config-ure the
  * Solution to the inner Block. If \p cfg is a
  * SimpleConfiguration< std::pair< std::string , Configuration * > >, then
  * cfg->first is used to fill the f_scenario_solution_fprefix field, which
  * then means that in deserialize() the Solution to the scenario Block is
  * written into files with that as a prefix rather than as sub-groups; see
  * the comments to serialize(). */

 void set_inner_Config( Configuration * cfg ) {
  using SCPSCp = SimpleConfiguration< std::pair< std::string ,
                                                 Configuration * > >;
  if( auto sscfg = dynamic_cast< SCPSCp * >( cfg ) ) {
   f_scenario_solution_fprefix = sscfg->f_value.first;
   cfg = sscfg->f_value.second;
   }

  f_inner_Config = cfg;
  }

/*-------------------- PROTECTED PART OF THE CLASS -------------------------*/

 protected:

/*-------------------------- PROTECTED METHODS -----------------------------*/

 void print( std::ostream &output ) const override {
  output << "TwoStageStochasticBlockSolution [" << this << "]: " << std::endl;
  }

/*---------------------- PRIVATE PART OF THE CLASS -------------------------*/

 private:

/*---------------------------- PRIVATE FIELDS ------------------------------*/

 std::vector< std::vector< double > > v_here_and_now;
 ///< values of here_and_now variables, divided into their groups

 Configuration * f_inner_Config;
 ///< the Configuration for scenario Solution

 std::vector< Solution * > v_scenario_solutions;
 ///< solutions for each scenario

 std::string f_scenario_solution_fprefix;
 ///< the prefix for the filenames containing the scenario solutions

 /*!!
  boost::multi_array< double , 2 > v_dual_values;
  dual values of non_anticipaticity constraints
  */

/*--------------------------------------------------------------------------*/

 SMSpp_insert_in_factory_h;

/*--------------------------------------------------------------------------*/

 };  // end( class( TwoStageStochasticBlockSolution ) )

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

}  // end( namespace SMSpp_di_unipi_it )

/*--------------------------------------------------------------------------*/

#endif /* TwoStageStochasticBlock.h included */

/*--------------------------------------------------------------------------*/
/*----------------- End File TwoStageStochasticBlock.h ---------------------*/
/*--------------------------------------------------------------------------*/
