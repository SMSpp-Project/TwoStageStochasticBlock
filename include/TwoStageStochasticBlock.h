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

class TwoStageStochasticBlock : public Block
{
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
  *               TwoStageStochasticBlock. */

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
 /// de-serialize a TwoStageStochasticBlock out of netCDF::NcGroup
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

    if( auto ib = stochastic_block->get_inner_block() ) {
     // Scale the objective according to the current scenario probability
     // ib->scale( scenario_gen->get_current_scenario_probability() );
     // Add the sub-Block to the vector of blocks
     v_Block.push_back( ib );
    }
   } else
    throw( std::logic_error(
     "TwoStageStochasticBlock::deserialize: sub-Block is not a StochasticBlock." ) );

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

/** @} ---------------------------------------------------------------------*/
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

/** @} ---------------------------------------------------------------------*/
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
  *         given \p stage and having index \p sub_block_index. */

 virtual StochasticBlock * get_sub_Block( Index scenario ) const;

/*--------------------------------------------------------------------------*/
 /// returns the number of scenarios

 Index get_number_scenarios( void ) const { return( f_number_scenarios ); }

/*--------------------------------------------------------------------------*/
 /// returns the set of scenarios

 const ScenarioGenerator * get_scenario_generator( void ) const {
  return( scenario_gen );
  }

/*--------------------------------------------------------------------------*/
 /// get the paths to static here_and_now variables

 const std::vector< std::unique_ptr< AbstractPath > > &
                       get_paths_to_static_here_and_now_vars( void ) const {
  return( v_paths_to_static_vars );
  }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
 /// get the paths to static here_and_now variables

 const std::vector< std::unique_ptr< AbstractPath > > &
                       get_paths_to_dynamic_here_and_now_vars( void ) const {
  return( v_paths_to_dynamic_vars );
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

/** @} ---------------------------------------------------------------------*/
/*-------- METHODS FOR Saving THE DATA OF THE TwoStageStochasticBlock ------*/
/*--------------------------------------------------------------------------*/
/** @name Saving the data of the TwoStageStochasticBlock
 *  @{ */

/** @} ---------------------------------------------------------------------*/
/*----------------------- Methods for handling Solution --------------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for handling Solution
 * @{ */

 /// returns the current Solution of this TwoStageStochasticBlock
 /** This method must construct and return a (pointer to a) Solution object
  * containing the current "solution state" of this TwoStageStochasticBlock.
  * This is a TwoStageStochasticBlockSolution.
  *
  * The parameter for deciding which kind of Solution must be returned is a
  * single int "value", coded bitwise:
  *
  * - bit 0 (& 1) means "store the value of the here_and_now variables"
  *               (only one copy, since they are supposed to be all equal)
  *
  * - bit 1 (& 2) means "store the solutions of each scenario" (comprising
  *               all the copies of the here_and_now variables)
  *
  * - bit 2 (& 4) means "store the dual prices of the non-anticipativity
  *               constraints" [to be implemented]
  *
  * This value is to be found into a Configuration cfg, which is:
  *
  * - if solc is not nullptr, then cfg = solc
  *
  * - otherwise, if f_BlockConfig is not nullptr, then cfg =
  *   f_BlockConfig->f_solution_Configuration
  *
  * - otherwise, cfg == nullptr
  *
  * Then:
  *
  * - if cfg is a SimpleConfiguration< int >, then value = cfg->f_value;
  *
  * - if cfg is a SimpleConfiguration< std::pair< int , Configuration * > >,
  *   then value = cfg->f_value.first
  *
  * - otherwise, value = 1 (only save the here_and_now variables).
  *
  * In case value & 2 is nonzero, the Solution objects for each scenario in
  * principle need a Configuration. This is supported by the case where
  * cfg is a SimpleConfiguration< std::pair< int , Configuration * > >, with
  * the Configuration obviously being (pointed by) cfg->f_value.second. In
  * all other cases the Configuration will be nullptr (the default one).
  * This Configuration is passed to the TwoStageStochasticBlockSolution via
  * the set_inner_Config() method, which also allows to set the format of
  * that Solution (see the comments). */

 Solution * get_Solution( Configuration * solc = nullptr ,
                          bool emptys = true ) override;

/** @} ---------------------------------------------------------------------*/
/*-------------------- Methods for handling Modification -------------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for handling Modification
 *  @{ */

 void add_Modification( sp_Mod mod , ChnlName chnl = 0 ) override;

/** @} ---------------------------------------------------------------------*/
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

 Index f_number_scenarios{};         ///< The number of scenarios

 ScenarioGenerator * scenario_gen;   ///< The scenario generator

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

  return( StochasticBlock_block );
 }

};   // end( class TwoStageStochasticBlock )

/*--------------------------------------------------------------------------*/
/*----------------- CLASS TwoStageStochasticBlockSolution ------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------- GENERAL NOTES --------------------------------*/
/*--------------------------------------------------------------------------*/
/// a Solution of a TwoStageStochasticBlock
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

 ~TwoStageStochasticBlockSolution() {
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

#endif  /* TwoStageStochasticBlock.h included */

/*--------------------------------------------------------------------------*/
/*----------------- End File TwoStageStochasticBlock.h ---------------------*/
/*--------------------------------------------------------------------------*/
