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
               const string& fBody)
{
	string func;
	if (!retType || (*retType)->isStructureType()) {
		func = "void " + fName + "(void){\n";
	} else if ((*retType)->isArrayType()) {
		// TODO: What about arrays of anonymous types?
		func = genVarDecl(context->getArrayDecayedType(*retType), fName + "(void)") + "{\nreturn ";
	} else {
		string decl = genVarDecl(*retType, fName + "(void)");
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
