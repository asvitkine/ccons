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

std::string genFunc(const clang::QualType *retType,
                    const string& fName,
                    const string& fBody)
{
	string func;
	if (!retType) {
		func = "void " + fName + "(void){\n";
	} else {
		func = genVarDecl(*retType, fName + "(void)") + "{\nreturn ";
	}
	func += fBody;
	func += "\n}\n";
	return func;
}

} // namespace ccons
