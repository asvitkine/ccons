cmake_minimum_required(VERSION 2.6)

set(LLVM_NO_RTTI 1)

find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config PATHS ../../Debug/bin/)
if(NOT LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "LLVM was not found!")
endif(NOT LLVM_CONFIG_EXECUTABLE)

exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --libs engine backend linker bitreader bitwriter OUTPUT_VARIABLE LLVM_LIBS1)
set(LLVM_LIBS "-ldl ${LLVM_LIBS1}")
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS)
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR)
set(CLANG_LIBS "-lclangFrontend -lclangAST -lclangLex -lclangCodeGen -lclangSema -lclangParse -lclangAST -lclangBasic")

Project(ccons)

add_executable(ccons ccons.cpp Diagnostics.cpp ClangUtils.cpp Console.cpp Parser.cpp SrcGen.cpp StringUtils.cpp EditLineReader.cpp InternalCommands.cpp LineReader.cpp RemoteConsole.cpp Visitors.cpp popen2.c)

include_directories(${LLVM_INCLUDE_DIR})
include_directories(../../include/)
include_directories(../clang/include/)

set_target_properties(ccons PROPERTIES LINK_FLAGS "${LLVM_LDFLAGS} -ldl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -fno-rtti")

set(LIBS ${LIBS} ${CLANG_LIBS})
set(LIBS ${LIBS} ${LLVM_LIBS})
set(LIBS ${LIBS} -ledit -lcurses)
target_link_libraries(ccons ${LIBS})

