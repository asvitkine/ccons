#ifndef CCONS_SRC_GEN_H
#define CCONS_SRC_GEN_H

#include <string>

namespace clang {
	class ASTContext;
	class QualType;
} // namespace clang

namespace ccons {

class Parser;

std::string genVarDecl(const clang::QualType& type, const std::string& vName);
std::string genFunc(const clang::QualType *retType,
                    clang::ASTContext *context,
                    const std::string& fName,
                    const std::string& fBody);

} // namespace ccons

#endif // CCONS_SRC_GEN_H
