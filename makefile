##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of TwoStageStochasticBlock                                      #
#                                                                            #
#   Note that $(SMS++INC) is assumed to include any -I directive             #
#   corresponding to external libraries needed by SMS++, at least to the     #
#   extent in which they are needed by the parts of SMS++ used by            #
#   TwoStageStochasticBlock.                                                 #
#                                                                            #
#   Input:  $(CC)          = compiler command                                #
#           $(SW)          = compiler options                                #
#           $(SMS++INC)    = the -I$( core SMS++ directory )                 #
#           $(SMS++OBJ)    = the libSMS++ library itself                     #
#           $(StcBlkH)     = the .h files to include for StochasticBlock     #
#           $(StcBlkINC)   = the -I$( StochasticBlock source directory )     #
#           $(TSSBkSDR)    = the directory where the source is               #
#                                                                            #
#   Output: $(TSSBkOBJ)    = the final object(s) / library                   #
#           $(TSSBkH)      = the .h files to include                         #
#           $(TSSBkINC)    = the -I$( source directory )                     #
#                                                                            #
#                              Antonio Frangioni                             #
#                         Dipartimento di Informatica                        #
#                             Universita' di Pisa                            #
#                                                                            #
##############################################################################


# macros to be exported - - - - - - - - - - - - - - - - - - - - - - - - - - -

TSSBkOBJ = $(TSSBkSDR)/obj/TwoStageStochasticBlock.o

TSSBkINC = -I$(TSSBkSDR)/include

TSSBkH   = $(TSSBkSDR)/include/TwoStageStochasticBlock.h

# clean - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -f $(TSSBkOBJ) $(MILPBSDR)/*~

# dependencies: every .o from its .cpp + every recursively included .h- - - -

$(TSSBkSDR)/obj/TwoStageStochasticBlock.o: \
	$(TSSBkSDR)/src/TwoStageStochasticBlock.cpp \
	$(TSSBkSDR)/include/TwoStageStochasticBlock.h $(StcBlkH) $(SMS++OBJ)
	$(CC) -c $(TSSBkSDR)/src/TwoStageStochasticBlock.cpp -o $@ \
	$(TSSBkINC) $(StcBlkINC) $(SMS++INC) $(SW)

########################## End of makefile ###################################
