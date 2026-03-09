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

if (WIN32 AND BUILD_SHARED_LIBS)
    # copy runtime DLLs next to the exe after build
    add_custom_command(TARGET TwoStageStochasticBlock_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:TwoStageStochasticBlock_test>
            $<TARGET_FILE_DIR:TwoStageStochasticBlock_test>
            COMMAND_EXPAND_LISTS
            COMMENT "Copying runtime DLLs next to TwoStageStochasticBlock_test.exe"
    )
endif()

add_test(NAME TwoStageStochasticBlock_test
         COMMAND TwoStageStochasticBlock_test)

# --------------------------------------------------------------------------- #
