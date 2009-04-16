#ifndef CCONS_CLANG_UTILS_H
#define CCONS_CLANG_UTILS_H

#include <llvm/ADT/OwningPtr.h>

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/DeclGroup.h>

namespace clang {
	class Diagnostic;
	class SourceManager;
} // namespace clang


namespace ccons {

class ProxyDiagnosticClient : public clang::DiagnosticClient {

public:

	ProxyDiagnosticClient(clang::DiagnosticClient *DC);

	void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
	                      const clang::DiagnosticInfo &Info);

	bool hadErrors() const;

private:

	clang::DiagnosticClient *_DC;
	bool _hadErrors;

};

typedef std::pair<unsigned, unsigned> SrcRange;

SrcRange getStmtRange(const clang::Stmt *S,
                      const clang::SourceManager& sm,
                      const clang::LangOptions options);
SrcRange getStmtRangeWithSemicolon(const clang::Stmt *S,
                                   const clang::SourceManager& sm,
                                   const clang::LangOptions options);

} // namespace ccons

#endif // CCONS_CLANG_UTILS_H
