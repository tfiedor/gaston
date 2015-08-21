cmake_minimum_required(VERSION 2.8)
project(dWiNA2014)

set(REQUIRED_G++_VERSION 4.8.0)
# forcing 4.8 compilers
set(CMAKE_C_COMPILER "gcc")
set(GCC_ALTERNATIVES g++ g++-4.8)
#set(CMAKE_CXX_COMPILER "g++")

include_directories(src)
include_directories(src/app)
include_directories(src/app/Frontend)
include_directories(include)

add_executable(dWiNA
	src/app/main.cpp
	src/app/Frontend/ast.cpp
	src/app/Frontend/ast_visitor.cpp
	src/app/Frontend/astdump.cpp
	src/app/Frontend/code.cpp
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
	src/app/DecisionProcedure/decision_procedures.cpp
	src/app/DecisionProcedure/backward_decision_procedure.cpp
	src/app/DecisionProcedure/containers/VarToTrackMap.cpp
	src/app/DecisionProcedure/containers/StateSet.cpp
	src/app/DecisionProcedure/visitors/PrettyPrinter.cpp
)

link_directories(${CMAKE_SOURCE_DIR}/src/libs/)
target_link_libraries(dWiNA ${CMAKE_SOURCE_DIR}/src/libs/libmonabdd.so)
target_link_libraries(dWiNA ${CMAKE_SOURCE_DIR}/src/libs/libmonadfa.so)
target_link_libraries(dWiNA ${CMAKE_SOURCE_DIR}/src/libs/libmonagta.so)
target_link_libraries(dWiNA ${CMAKE_SOURCE_DIR}/src/libs/libmonamem.so)
target_link_libraries(dWiNA ${CMAKE_SOURCE_DIR}/src/libs/libvata.a)

if (UNIX)
	message(STATUS "Setting G++ flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_C++_FLAGS} -m32 -std=c++0x -DHAVE_CONFIG_H -I. -I.. -I../include -O2 -g -O3")
else()
	message(fatal_error "Not supported yet")
endif()
