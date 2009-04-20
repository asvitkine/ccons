//
// Utility functions for re-writing and generating strings with C code
// for various use cases.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "SrcGen.h"

#include <clang/AST/AST.h>

#include "Parser.h"

using std::string;

namespace ccons {

// Properly handles function pointer variables.
string genVarDecl(const clang::QualType& type, const string& vName) {
	string str = vName;
	type.getUnqualifiedType().getAsStringInternal(str);
	return str;
}

string genFunc(const clang::QualType *retType,
               clang::ASTContext *context,
               const string& fName,
               const string& fBody,
               int& bodyOffset)
{
	string func;
	if (!retType || (*retType)->isStructureType()) {
		func = "void " + fName + "(void){\n";
	} else if ((*retType)->isArrayType()) {
		// TODO: What about arrays of anonymous types?
		func = genVarDecl(context->getArrayDecayedType(*retType), fName + "(void)") + "{\nreturn ";
	} else {
		// prefix is used to promoting a function to a function pointer for
		// the return value (this conversion is done automatically in C)
		string prefix = ((*retType)->isFunctionType() ? "*" : "");
		string decl = genVarDecl(*retType, prefix + fName + "(void)");
		// TODO: check for anonymous struct a better way
		if (decl.find("struct <anonymous>") != string::npos) {
			if ((*retType)->isPointerType()) {
				func = "void *" + fName + "(void){\nreturn ";
			} else {
				func = "void " + fName + "(void){\n";
			}
		} else {
			func = decl + "{\nreturn ";
		}
	}
	bodyOffset = func.length();
	func += fBody;
	func += "\n}\n";
	return func;
}

std::string getFunctionDeclAsString(const clang::FunctionDecl *FD)
{
	const clang::FunctionType *FT = FD->getType()->getAsFunctionType();
	string str = FD->getNameAsString();
	FT->getAsStringInternal(str);
	str += ";";
	return str;
}

} // namespace ccons
