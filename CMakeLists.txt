set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
#include(cotire)

cmake_minimum_required(VERSION 2.8)
project(gaston2015)

set(REQUIRED_G++_VERSION 4.8.0)
# forcing 4.8 compilers
set(CMAKE_C_COMPILER "gcc")
set(GCC_ALTERNATIVES g++ g++-4.8)
set(CMAKE_CXX_COMPILER "g++-4.9")

include_directories(src)
include_directories(src/app)
include_directories(src/app/Frontend)
include_directories(include)

add_executable(gaston
	src/app/main.cpp
	src/app/Frontend/ast.cpp
	src/app/Frontend/ast_visitor.cpp
	src/app/Frontend/astdump.cpp
	src/app/Frontend/code.cpp
	src/app/Frontend/code_visitor.cpp
	src/app/Frontend/codedump.cpp
	src/app/Frontend/codesubst.cpp
	src/app/Frontend/codetable.cpp
	src/app/Frontend/freevars.cpp
	src/app/Frontend/ident.cpp
	src/app/Frontend/lib.cpp
	src/app/Frontend/makeguide.cpp
	src/app/Frontend/offsets.cpp
	src/app/Frontend/parser.cpp
	src/app/Frontend/predlib.cpp
	src/app/Frontend/printline.cpp
	src/app/Frontend/reduce.cpp
	src/app/Frontend/scanner.cpp
	src/app/Frontend/signature.cpp
	src/app/Frontend/st_dfa.cpp
	src/app/Frontend/st_gta.cpp
	src/app/Frontend/symboltable.cpp
	src/app/Frontend/timer.cpp
	src/app/Frontend/untyped.cpp
	src/app/DecisionProcedure/automata.cpp
	src/app/DecisionProcedure/formula_existential_form_conversion.cpp
	src/app/DecisionProcedure/formula_flattening.cpp
	src/app/DecisionProcedure/formula_to_automaton_ws1s.cpp
	src/app/DecisionProcedure/formula_to_automaton_ws2s.cpp
	src/app/DecisionProcedure/formula_to_symbolic_automaton.cpp
	src/app/DecisionProcedure/checkers/Checker.cpp
	src/app/DecisionProcedure/checkers/SymbolicChecker.cpp
	src/app/DecisionProcedure/containers/VarToTrackMap.cpp
	src/app/DecisionProcedure/containers/SymbolicAutomata.cpp
	src/app/DecisionProcedure/containers/Term.cpp
	src/app/DecisionProcedure/containers/Workshops.cpp
	src/app/DecisionProcedure/containers/FixpointGuide.cpp
	src/app/DecisionProcedure/containers/TermEnumerator.cpp
	src/app/DecisionProcedure/utils/Symbol.cpp
	src/app/DecisionProcedure/visitors/restricters/Flattener.cpp
	src/app/DecisionProcedure/visitors/restricters/NegationUnfolder.cpp
	src/app/DecisionProcedure/visitors/restricters/SecondOrderRestricter.cpp
	src/app/DecisionProcedure/visitors/restricters/SyntaxRestricter.cpp
	src/app/DecisionProcedure/visitors/restricters/UniversalQuantifierRemover.cpp
	src/app/DecisionProcedure/visitors/restricters/PredicateUnfolder.cpp
	src/app/DecisionProcedure/visitors/restricters/ZeroOrderRemover.cpp
	src/app/DecisionProcedure/visitors/transformers/BooleanUnfolder.cpp
	src/app/DecisionProcedure/visitors/transformers/PrenexNormalFormTransformer.cpp
	src/app/DecisionProcedure/visitors/transformers/AntiPrenexer.cpp
	src/app/DecisionProcedure/visitors/transformers/Reorderer.cpp
	src/app/DecisionProcedure/visitors/transformers/BinaryReorderer.cpp
	src/app/DecisionProcedure/visitors/transformers/QuantificationMerger.cpp
	src/app/DecisionProcedure/visitors/transformers/BaseAutomataMerger.cpp
	src/app/DecisionProcedure/visitors/transformers/ExistentialPrenexer.cpp
	src/app/DecisionProcedure/visitors/transformers/ShuffleVisitor.cpp
	src/app/DecisionProcedure/visitors/transformers/ContinuationSwitcher.cpp
	src/app/DecisionProcedure/visitors/decorators/FixpointDetagger.cpp
	src/app/DecisionProcedure/visitors/decorators/InverseFixpointDetagger.cpp
	src/app/DecisionProcedure/visitors/decorators/Tagger.cpp
	src/app/DecisionProcedure/visitors/decorators/OccuringVariableDecorator.cpp
	src/app/DecisionProcedure/visitors/decorators/UnderComplementDecorator.cpp
	src/app/DecisionProcedure/visitors/printers/DotWalker.cpp
	src/app/DecisionProcedure/visitors/printers/MonaAutomataDotWalker.cpp
	src/app/DecisionProcedure/visitors/printers/MonaSerializer.cpp
)

link_directories(${CMAKE_SOURCE_DIR}/src/libs/)
target_link_libraries(gaston ${CMAKE_SOURCE_DIR}/src/libs/libmonabdd.so)
target_link_libraries(gaston ${CMAKE_SOURCE_DIR}/src/libs/libmonadfa.so)
target_link_libraries(gaston ${CMAKE_SOURCE_DIR}/src/libs/libmonagta.so)
target_link_libraries(gaston ${CMAKE_SOURCE_DIR}/src/libs/libmonamem.so)
target_link_libraries(gaston ${CMAKE_SOURCE_DIR}/src/libs/libvata.a)
#cotire(gaston)

if (UNIX)
	message(STATUS "Setting G++ flags")
	#set(CMAKE_CXX_FLAGS "${CMAKE_C++_FLAGS} -coverage -std=c++0x -DHAVE_CONFIG_H -I. -I.. -I../include -O2 -g -O3")
	set(CMAKE_CXX_FLAGS "${CMAKE_C++_FLAGS} -std=c++0x -pthread -DHAVE_CONFIG_H -I. -I.. -I../include -O2 -g -O3 -DBOOST_SYSTEM_NO_DEPRECATED")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lpthread -lboost_system -DBOOST_SYSTEM_NO_DEPRECATED")
else()
	message(fatal_error "Not supported yet")
endif()

##############################################################################
#                                   TESTS
##############################################################################

# Include CTest so that sophisticated testing can be done now
include(CTest)

add_test(regression ${PROJECT_SOURCE_DIR}/testcheck.py)
