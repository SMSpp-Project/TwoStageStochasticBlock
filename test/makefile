##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of test (for TwoStageStochasticBlock deserialization)          #
#                                                                            #
##############################################################################

# module name
NAME = TwoStageStochasticBlock_test
FACTORY_TEST = test_factory

# basic directory
DIR = .

# compiler
CC = clang++

# common flags
COMMON_SW = -std=c++20 -ferror-limit=1 -Wno-enum-compare -DCLANG_1200_0_32_27_PATCH

# debug switches
SW_DEBUG = -g3 -glldb -fno-inline $(COMMON_SW)

# production switches
SW_RELEASE = -O3 $(COMMON_SW) -DNDEBUG

# default target - build in release mode
default: SW = $(SW_RELEASE)
default: $(DIR)/$(NAME)

# factory test target
factory: SW = $(SW_RELEASE)
factory: $(DIR)/$(FACTORY_TEST)

# debug target - build in debug mode
debug: SW = $(SW_DEBUG)
debug: $(DIR)/$(NAME)

# define & include the necessary modules- - - - - - - - - - - - - - - - - - -
# if a module is not used in the current configuration, just comment out the
# corresponding include line
# each module outputs some macros to be used here:
# *OBJ is the final object(s) / library
# *LIB is the external libraries + -L< libdirs >
# *H   is the list of all include files
# *INC is the -I< include directories >

# SMS++
SMS++SDR = ../../SMS++
include $(SMS++SDR)/lib/makefile-inc

# TwoStageStochasticBlock
TSSBkSDR = ..
include $(TSSBkSDR)/makefile-s

# main module (linking phase) - - - - - - - - - - - - - - - - - - - - - - - -

# object files
MOBJ =  $(TSSBkOBJ) $(SMS++OBJ)

# libraries (need to explicitly include dependencies' libraries since TSSBkLIB is empty)
MLIB =  $(TSSBkLIB) $(StcBlkLIB) $(SMS++LIB)

$(DIR)/$(NAME): $(MOBJ) $(DIR)/test.o
	$(CC) -o $(DIR)/$(NAME) $(DIR)/test.o $(MOBJ) $(MLIB) $(SW)

# dependencies: every .o from its .cpp + every recursively included .h- - - -

# include directives
MINC =  $(SMS++INC) $(TSSBkINC)

# includes
MH =    $(SMS++H) $(TSSBkH)

# compile command

$(DIR)/test.o: $(DIR)/test.cpp $(MH)
	$(CC) -c $*.cpp -o $@ $(MINC) $(SW)

# factory test
$(DIR)/$(FACTORY_TEST): $(MOBJ) $(DIR)/test_factory.o
	$(CC) -o $(DIR)/$(FACTORY_TEST) $(DIR)/test_factory.o $(MOBJ) $(MLIB) $(SW)

$(DIR)/test_factory.o: $(DIR)/test_factory.cpp $(MH)
	$(CC) -c $(DIR)/test_factory.cpp -o $@ $(MINC) $(SW)

# clean target
clean::
	rm -f $(DIR)/*.o $(DIR)/*~
	rm -f $(DIR)/$(NAME)

# phony targets
.PHONY: default debug clean factory

############################ End of makefile #################################
