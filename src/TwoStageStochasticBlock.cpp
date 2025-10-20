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

// register TwoStageStochasticBlock to the Block factory
SMSpp_insert_in_factory_cpp_1( TwoStageStochasticBlock );

// register TwoStageStochasticBlockSolution to the Solution factory
SMSpp_insert_in_factory_cpp_0( TwoStageStochasticBlockSolution );

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
 if( constraints_generated() )  // constraints have already been generated
  return;                       // nothing to do

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
/*----------------------- Methods for handling Solution --------------------*/
/*--------------------------------------------------------------------------*/

Solution * TwoStageStochasticBlock::get_Solution( Configuration * solc ,
						  bool emptys )
{
 Index wsol = 1;
 auto * sol = new TwoStageStochasticBlockSolution;

 if( ( ! solc ) && f_BlockConfig )
  solc = f_BlockConfig->f_solution_Configuration;

 if( auto config = dynamic_cast< SimpleConfiguration< int > * >( solc ) )
  wsol = config->f_value;
 else
  if( auto config =
      dynamic_cast< SimpleConfiguration< std::pair< int , Configuration * >
                                         > * >( solc ) ) {
   wsol = config->f_value.first;
   sol->set_inner_Config( config->f_value.second );
   }

 if( wsol & 1 )
  sol->v_here_and_now.resize( v_paths_to_static_vars.size() );

 if( wsol & 2 )
  sol->v_scenario_solutions.resize( get_number_scenarios() );

 if( wsol & 4 )
  throw( std::invalid_argument( "TwoStageStochasticBlock::get_Solution: "
				"saving of dual variables not implemented yet"
				) );

 return( sol );

 }  // end( TwoStageStochasticBlock::get_Solution )

/*--------------------------------------------------------------------------*/
/*------ METHODS DESCRIBING THE BEHAVIOR OF A TwoStageStochasticBlock ------*/
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
/*------------- METHODS OF TwoStageStochasticBlockSolution -----------------*/
/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlockSolution::deserialize(
					      const netCDF::NcGroup & group )
{
 // call the method of the base class - not, it does nothing
 // Solution::deserialize( group );

 // read dimensions- - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 int tNHaN = 0;  // total number of here_and_now variables
 deserialize_dim( group , "TotalNumberHereAndNow" , tNHaN , true );
 
 int NHaNG = 1;   // number of here_and_now variables groups
 deserialize_dim( group , "NumberHereAndNowGroups" , NHaNG , true );

 // read here_and_now variables- - - - - - - - - - - - - - - - - - - - - - -
 // if "HereAndNow" is not there, v_here_and_now.empty() == true
 if( tNHaN ) {
  if( NHaNG == 1 ) {  // it's a vector
   v_here_and_now.resize( 1 );
   ::deserialize< double >( group , "HereAndNow" , v_here_and_now[ 0 ] ,
			    true );
   if( v_here_and_now[ 0 ].empty() )
    v_here_and_now.clear();
   }
  else                // it's a vector of vectors
   ::deserialize< double >( group , "HereAndNow" , "HereAndNowStart" ,
			    v_here_and_now , true );
  }

 // read scenario solutions- - - - - - - - - - - - - - - - - - - - - - - - -
 int NS = 0;     // number of scenarios
 deserialize_dim( group , "NumberScenarios" , NS , true );

 if( NS ) {
  ::deserialize( group , f_scenario_solution_fprefix ,
		 "ScenarioSolutionPrefix" , true );

  if( f_scenario_solution_fprefix.empty() ) {
   for( int i = 0 ; i < NS ; ++i ) {
    std::string sgn = "ScenarioSolution_" + std::to_string( i );
    auto sg = group.getGroup( sgn );
    if( sg.isNull() ) {
     if( ! i )
      break;
     else
      throw( std::invalid_argument(
			  "TwoStageStochasticBlockSolution::deserialize: "
                          "group" + sgn + " not present" ) );
     }
    if( ! i )
     v_scenario_solutions.resize( NS );
    if( auto si = Solution::new_Solution( sg ) )
     v_scenario_solutions[ i ] = si;
    else
     throw( std::invalid_argument(
			  "TwoStageStochasticBlockSolution::deserialize: "
			  " deserialize from group " + sgn + " failed" ) );
    }
   }
  else
   for( int i = 0 ; i < NS ; ++i ) {
    std::string sfn = f_scenario_solution_fprefix + "_" +
                      std::to_string( i ) + ".nc4";
    if( auto si = Solution::deserialize( sfn ) )
     v_scenario_solutions[ i ] = si;
    else
     throw( std::invalid_argument(
			  "TwoStageStochasticBlockSolution::deserialize: "
			  " deserialize from file " + sfn + "failed" ) );
    }
  }

 // read the dual variables of non-anticipativity constraints - - - - - - - -
 //!! TODO

 }  // end( TwoStageStochasticBlockSolution::deserialize )

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlockSolution::read( const Block * block )
{
 auto TSSB = dynamic_cast< const TwoStageStochasticBlock * >( block );
 if( ! TSSB )
  throw( std::invalid_argument( "TwoStageStochasticBlockSolution::read: "
				"block is not a TwoStageStochasticBlock" ) );

 // read here_and_now variables- - - - - - - - - - - - - - - - - - - - - - -
 if( ! v_here_and_now.empty() ) {
  auto b0 = TSSB->get_sub_Block( 0 );
  auto & ptshanv = TSSB->get_paths_to_static_here_and_now_vars();
  v_here_and_now.resize( ptshanv.size() );
  for( std::size_t i = 0 ; i < ptshanv.size() ; ++i ) {
   auto nv = ptshanv[ i ]->get_number_elements< ColVariable >( b0 );
   v_here_and_now[ i ].resize( nv );
   auto * elem = ptshanv[ i ]->get_element< ColVariable >( b0 );
   for( std::size_t j = 0 ; j < nv ; ++j , ++elem )
    v_here_and_now[ i ][ j ] = elem->get_value();
   }
  }

 // read scenario solutions- - - - - - - - - - - - - - - - - - - - - - - - -
 if( ! v_scenario_solutions.empty() ) {
  auto ns = TSSB->get_number_scenarios();
  v_scenario_solutions.resize( ns , nullptr );
  for( Block::Index i = 0 ; i < ns ; ++i ) {
   delete v_scenario_solutions[ i ];
   v_scenario_solutions[ i ] =
                    TSSB->get_sub_Block( i )->get_Solution( f_inner_Config );
   }
  }

 // read the dual variables of non-anticipativity constraints - - - - - - - -
 //!! TODO

 }  // end( TwoStageStochasticBlockSolution::read )

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlockSolution::write( Block * block )
{
 auto TSSB = dynamic_cast< TwoStageStochasticBlock * >( block );
 if( ! TSSB )
  throw( std::invalid_argument( "TwoStageStochasticBlockSolution::write: "
				"block is not a TwoStageStochasticBlock" ) );

 // write here_and_now variables - - - - - - - - - - - - - - - - - - - - - -
 if( ! v_here_and_now.empty() ) {
  auto b0 = TSSB->get_sub_Block( 0 );
  auto & ptshanv = TSSB->get_paths_to_static_here_and_now_vars();
  if( v_here_and_now.size() != ptshanv.size() )
   throw( std::invalid_argument( "TwoStageStochasticBlockSolution::write: "
				 "inconsistent number of groups of h&n "
				 "variables" ) );

  for( std::size_t i = 0 ; i < ptshanv.size() ; ++i ) {
   auto nv = ptshanv[ i ]->get_number_elements< ColVariable >( b0 );
   if( v_here_and_now[ i ].size() != nv )
    throw( std::invalid_argument( "TwoStageStochasticBlockSolution::write: "
				  "inconsistent number of h&n variables "
				  "in group " + std::to_string( i ) ) );

   auto * elem = ptshanv[ i ]->get_element< ColVariable >( b0 );
   for( Block::Index j = 0 ; j < nv ; ++j , ++elem )
    elem->set_value( v_here_and_now[ i ][ j ] );
   }
  }

 // write scenario solutions - - - - - - - - - - - - - - - - - - - - - - - -
 if( ! v_scenario_solutions.empty() ) {
  auto ns = TSSB->get_number_scenarios();
  if( v_scenario_solutions.size() != ns )
   throw( std::invalid_argument( "TwoStageStochasticBlockSolution::write: "
				 "inconsistent scenarios number" ) );

  for( Block::Index i = 0 ; i < ns ; ++i )
   v_scenario_solutions[ i ]->write( TSSB->get_sub_Block( i ) );
  }

 // write the dual variables of non-anticipativity constraints- - - - - - - -
 //!! TODO

 }  // end( TwoStageStochasticBlockSolution::write )

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlockSolution::serialize( netCDF::NcGroup & group )
 const
{
 // call the method of the base class
 Solution::serialize( group );

 // serialize the here_and_now variables- - - - - - - - - - - - - - - - - - -
 if( ! v_here_and_now.empty() ) {
  int tmnhan = 0;
  for( auto hani : v_here_and_now )
   tmnhan += hani.size();

  auto TNHaN = group.addDim( "TotalNumberHereAndNow" , tmnhan );

  if( v_here_and_now.size() == 1 )
   ::serialize< double >( group , "HereAndNow" , netCDF::NcDouble() ,
			  TNHaN , v_here_and_now[ 0 ] );
  else {
   auto NHaNG = group.addDim( "NumberHereAndNowGroups" ,
			      v_here_and_now.size() );

   ::serialize< double >( group , "HereAndNow" , netCDF::NcDouble() ,
			  "HereAndNowStart" , v_here_and_now ,
			  TNHaN , NHaNG );
   }
  }

 // serialize the scenario solutions - - - - - - - - - - - - - - - - - - - -
 if( ! v_scenario_solutions.empty() ) {
  auto NS = group.addDim( "NumberScenarios" , v_scenario_solutions.size() );

  if( f_scenario_solution_fprefix.empty() ) {  // group-based format
   for( std::size_t i = 0 ; i < v_scenario_solutions.size() ; ++i ) {
    std::string sgn = "ScenarioSolution_" + std::to_string( i );
    auto sg = group.addGroup( sgn );
    v_scenario_solutions[ i ]->serialize( sg );
    }
   }
  else {                                       // file-based format
   for( std::size_t i = 0 ; i < v_scenario_solutions.size() ; ++i ) {
    std::string sfn = f_scenario_solution_fprefix + "_" +
                      std::to_string( i ) + ".nc4";
    v_scenario_solutions[ i ]->serialize( sfn );
    }
   }
  }

 // serialize the scenario dual variables of non-anticipativity constraints - 
 //!! TODO

 }  // end( TwoStageStochasticBlockSolution::serialize( NcGroup & ) )

/*--------------------------------------------------------------------------*/

TwoStageStochasticBlockSolution * TwoStageStochasticBlockSolution::scale(
						        double factor ) const
{
 auto sol = clone();  // create a copy of this TwoStageStochasticBlockSolution

 if( factor == 1 )
  return( sol );

 for( auto & hani : sol->v_here_and_now )
  for( auto & hanij : hani )
   hanij *= factor;

 for( auto ssi : sol->v_scenario_solutions )
  ssi->scale( factor );

 //!! TODO: deal with v_dual_values

 return( sol );

 }  // end( TwoStageStochasticBlockSolution::scale )

/*--------------------------------------------------------------------------*/

void TwoStageStochasticBlockSolution::sum( const Solution * solution ,
					   double multiplier )
{
 auto TSSBS = dynamic_cast< const TwoStageStochasticBlockSolution * >(
								  solution );
 if( ! TSSBS )
  throw( std::invalid_argument( "TwoStageStochasticBlockSolution::sum: "
				"solution not a "
				"TwoStageStochasticBlockSolution" ) );

 if( v_here_and_now.size() != TSSBS->v_here_and_now.size() )
  throw( std::invalid_argument( "TwoStageStochasticBlockSolution::sum: "
				"inconsistent here-and-now groups" ) );

 for( std::size_t i = 0 ; i < v_here_and_now.size() ; ++i ) {
  if( v_here_and_now[ i ].size() != TSSBS->v_here_and_now[ i ].size() )
   throw( std::invalid_argument( "TwoStageStochasticBlockSolution::sum: "
				 "inconsistent here-and-now group "
				 + std::to_string( i ) ) );

  for( std::size_t j = 0 ; j < v_here_and_now[ i ].size() ; ++j )
   v_here_and_now[ i ][ j ] += multiplier * TSSBS->v_here_and_now[ i ][ j ];
  }

 if( v_scenario_solutions.size() != TSSBS->v_scenario_solutions.size() )
  throw( std::invalid_argument( "TwoStageStochasticBlockSolution::sum: "
				"inconsistent number of scenarios" ) );

 for( std::size_t i = 0 ; i < v_scenario_solutions.size() ; ++i )
  v_scenario_solutions[ i ]->sum( TSSBS->v_scenario_solutions[ i ] ,
				  multiplier );

 //!! TODO: deal with v_dual_values
 
 }  // end( TwoStageStochasticBlockSolution::sum )

/*--------------------------------------------------------------------------*/

TwoStageStochasticBlockSolution * TwoStageStochasticBlockSolution::clone(
							  bool empty ) const
{
 auto sol = new TwoStageStochasticBlockSolution();

 if( ! empty ) {
  sol->v_here_and_now = v_here_and_now;
  sol->v_scenario_solutions.resize( v_scenario_solutions.size() );
  for( std::size_t i = 0 ; i < v_scenario_solutions.size() ; ++i )
   sol->v_scenario_solutions[ i ] = v_scenario_solutions[ i ]->clone();
  //!! sol->v_dual_values = v_dual_values;
  }

 return( sol );

 }  // end( TwoStageStochasticBlockSolution::clone )

/*--------------------------------------------------------------------------*/
/*---------------- End File TwoStageStochasticBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
