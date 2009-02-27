#ifndef CCONS_CLANG_UTILS_H
#define CCONS_CLANG_UTILS_H

#include <llvm/ADT/OwningPtr.h>

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>

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


class StmtFinder : public clang::StmtVisitor<StmtFinder> {

public:

	explicit StmtFinder(unsigned pos, const clang::SourceManager& sm);
	~StmtFinder();

	void VisitChildren(clang::Stmt *S);
	void VisitStmt(clang::Stmt *S);
	clang::Stmt * getStmt();

private:

	unsigned _pos;
	const clang::SourceManager& _sm;
	clang::Stmt *_S;

};



class FunctionBodyConsumer : public clang::ASTConsumer {

public:

	explicit FunctionBodyConsumer(StmtFinder *SF);
	~FunctionBodyConsumer();

	void HandleTopLevelDecl(clang::Decl *D);
	bool seenFunction() const;

private:

	StmtFinder *_SF;
	bool _seenFunction;

};


} // namespace ccons

#endif // CCONS_CLANG_UTILS_H
