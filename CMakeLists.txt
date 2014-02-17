cmake_minimum_required(VERSION 2.8)
project(dip2014)
include_directories(src)
include_directories(src/app)
include_directories(src/app/Frontend)
include_directories(libs)
add_executable(dip
	src/app/main.cpp
	src/app/Frontend/ast.cpp
	src/app/Frontend/astdump.cpp
	src/app/Frontend/code.cpp
	src/app/Frontend/codedump.cpp
	src/app/Frontend/codesubst.cpp
	src/app/Frontend/codetable.cpp
	src/app/Frontend/expnf.cpp
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
)
link_directories(${CMAKE_SOURCE_DIR}/src/libs/)
target_link_libraries(dip ${CMAKE_SOURCE_DIR}/src/libs/libmonabdd.so)
target_link_libraries(dip ${CMAKE_SOURCE_DIR}/src/libs/libmonadfa.so)
target_link_libraries(dip ${CMAKE_SOURCE_DIR}/src/libs/libmonagta.so)
target_link_libraries(dip ${CMAKE_SOURCE_DIR}/src/libs/libmonamem.so)
target_link_libraries(dip ${CMAKE_SOURCE_DIR}/src/libs/libvata.a)

if (UNIX)
	message(status "Setting G++ flags")
	set(CMAKE_C++_FLAGS "${CMAKE_C++_FLAGS} -DHAVE_CONFIG_H -I. -I.. -O2 -g -O3")
else()
	message(fatal_error "Not supported yet")
endif()