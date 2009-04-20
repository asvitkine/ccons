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
	class ASTContext;
	class QualType;
	class FunctionDecl;
} // namespace clang

namespace ccons {

class Parser;

std::string genVarDecl(const clang::QualType& type, const std::string& vName);
std::string genFunc(const clang::QualType *retType,
                    clang::ASTContext *context,
                    const std::string& fName,
                    const std::string& fBody,
                    int& bodyOffset);
std::string getFunctionDeclAsString(const clang::FunctionDecl *FD);

} // namespace ccons

#endif // CCONS_SRC_GEN_H
