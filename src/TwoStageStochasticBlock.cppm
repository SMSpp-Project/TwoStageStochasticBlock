/*--------------------------------------------------------------------------*/
/*------------------- File TwoStageStochasticBlock.cppm --------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Interface of the C++20 module smspp.twostagestochasticblock, which exports
 * every name that the headers of TwoStageStochasticBlock declare at namespace
 * scope, together with the modules it depends on; a translation unit can then
 * write
 *
 *     import smspp.twostagestochasticblock;
 *
 * in place of the #include of the headers. The macros (e.g., those of the
 * factory in SMSFactory.h) are not exported by a module, so a file that uses
 * them still includes the header that defines them.
 *
 * This file is written by SMS++/cmake/smspp_module_exports.py, which reads
 * the names from the AST of the headers: it is not to be edited by hand.
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
/*------------------------- GLOBAL MODULE FRAGMENT -------------------------*/
/*--------------------------------------------------------------------------*/

module;

#include "TwoStageStochasticBlock.h"

/*--------------------------------------------------------------------------*/
/*----------------------------- MODULE PURVIEW -----------------------------*/
/*--------------------------------------------------------------------------*/

export module smspp.twostagestochasticblock;

export import smspp.stochasticblock;

export namespace SMSpp_di_unipi_it {

 // TwoStageStochasticBlock.h
 using SMSpp_di_unipi_it::TwoStageStochasticBlock;
 using SMSpp_di_unipi_it::TwoStageStochasticBlockSolution;
}

/*--------------------------------------------------------------------------*/
/*----------------- End File TwoStageStochasticBlock.cppm ------------------*/
/*--------------------------------------------------------------------------*/
