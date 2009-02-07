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
	if (!retType) {
		func = "void " + fName + "(void){\n";
	} else if ((*retType)->isArrayType()) {
		func = genVarDecl(context->getArrayDecayedType(*retType), fName + "(void)") + "{\nreturn ";
	} else {
		func = genVarDecl(*retType, fName + "(void)") + "{\nreturn ";
	}
	func += fBody;
	func += "\n}\n";
	return func;
}

} // namespace ccons
