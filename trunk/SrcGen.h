#ifndef CCONS_SRC_GEN_H
#define CCONS_SRC_GEN_H

//
// Header for SrcGen.cpp, which provides various utility functions for
// re-writing and generating strings with C code for various use cases.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include <string>

namespace clang {
	struct PrintingPolicy;
	class ASTContext;
	class QualType;
	class FunctionDecl;
} // namespace clang

namespace ccons {

class Parser;

// Generate a variable declaration (like "int i;") for the specified type
// and variable name. Works for non-trivial types like function pointers.
std::string genVarDecl(const clang::PrintingPolicy& PP,
                       const clang::QualType& type,
                       const std::string& vName);

// Generate a function definition with the specified parameters. Returns
// the offset of the start of the body in variable bodyOffset.
std::string genFunction(const clang::PrintingPolicy& PP,
                        const clang::QualType *retType,
                        clang::ASTContext *context,
                        const std::string& fName,
                        const std::string& fBody,
                        int& bodyOffset);

// Get the function declaration as a string, from the FunctionDecl specified.
std::string getFunctionDeclAsString(const clang::PrintingPolicy& PP,
                                    const clang::FunctionDecl *FD);

} // namespace ccons

#endif // CCONS_SRC_GEN_H
