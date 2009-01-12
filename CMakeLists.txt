cmake_minimum_required(VERSION 2.6)

set(LLVM_NO_RTTI 1)

find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config PATHS /opt/local/bin)
if(NOT LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "LLVM was not found!")
endif(NOT LLVM_CONFIG_EXECUTABLE)

#exec_program(llvm-config ARGS --libs OUTPUT_VARIABLE LLVM_LIBS1)
#set(LLVM_LIBS "-ldl ${LLVM_LIBS1}")

#  /Users/shadowknight/Projects/llvm/Debug/lib/LLVMCClang.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMCBase.o
set(LLVM_LIBS "-ldl /Users/shadowknight/Projects/llvm/Debug/lib/LLVMXCore.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMSparcCodeGen.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMSparcAsmPrinter.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMPowerPCAsmPrinter.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMPowerPCCodeGen.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMPIC16.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMMSIL.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMMips.o -lLLVMLinker -lLLVMipo /Users/shadowknight/Projects/llvm/Debug/lib/LLVMInterpreter.o -lLLVMInstrumentation /Users/shadowknight/Projects/llvm/Debug/lib/LLVMIA64.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMExecutionEngine.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMJIT.o -lLLVMDebugger /Users/shadowknight/Projects/llvm/Debug/lib/LLVMCppBackend.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMCellSPUCodeGen.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMCellSPUAsmPrinter.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMCBackend.o -lLLVMBitWriter /Users/shadowknight/Projects/llvm/Debug/lib/LLVMX86AsmPrinter.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMX86CodeGen.o -lLLVMAsmParser /Users/shadowknight/Projects/llvm/Debug/lib/LLVMARMAsmPrinter.o /Users/shadowknight/Projects/llvm/Debug/lib/LLVMARMCodeGen.o -lLLVMArchive -lLLVMBitReader /Users/shadowknight/Projects/llvm/Debug/lib/LLVMAlphaCodeGen.o -lLLVMSelectionDAG /Users/shadowknight/Projects/llvm/Debug/lib/LLVMAlphaAsmPrinter.o -lLLVMAsmPrinter -lLLVMCodeGen -lLLVMScalarOpts -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMSupport -lLLVMSystem")
exec_program(llvm-config ARGS --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS)
exec_program(llvm-config ARGS --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR)
set(CLANG_LIBS "-lclangDriver -lclangAST -lclangLex -lclangCodeGen -lclangSema -lclangParse -lclangAST -lclangBasic")

Project(ccons)

add_executable(ccons ccons.cpp Console.cpp Parser.cpp)

include_directories(${LLVM_INCLUDE_DIR})
include_directories(../../include/)
include_directories(../clang/include/)

set_target_properties(ccons PROPERTIES LINK_FLAGS "${LLVM_LDFLAGS} -ldl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -fno-rtti")

set(LIBS ${LIBS} ${CLANG_LIBS} ${LLVM_LIBS})
set(LIBS ${LIBS} /usr/local/lib/libctemplate.a)
set(LIBS ${LIBS} /usr/local/lib/libedit.a -lcurses)
target_link_libraries(ccons ${LIBS})

