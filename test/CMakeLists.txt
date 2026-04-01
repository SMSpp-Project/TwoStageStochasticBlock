# --------------------------------------------------------------------------- #
#    CMake file for TwoStageStochasticBlock/tests                             #
#                                                                             #
#    In this file we define the unit tests for this library.                  #
#    Tests that should be supported by ctest executable must be added         #
#    with the add_test() command.                                             #
#                                                                             #
#                                Donato Meoli                                 #
#                         Dipartimento di Informatica                         #
#                             Universita' di Pisa                             #
# --------------------------------------------------------------------------- #

add_executable(TwoStageStochasticBlock_test test.cpp)
target_link_libraries(TwoStageStochasticBlock_test PRIVATE SMS++::TwoStageStochasticBlock)

add_test(NAME TwoStageStochasticBlock_test
         COMMAND TwoStageStochasticBlock_test)

# --------------------------------------------------------------------------- #
