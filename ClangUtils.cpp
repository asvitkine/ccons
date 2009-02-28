#include "ClangUtils.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Sema/SemaDiagnostic.h>

namespace ccons {

ProxyDiagnosticClient::ProxyDiagnosticClient(clang::DiagnosticClient *DC)
	: _DC(DC), _hadErrors(false)
{
}

void ProxyDiagnosticClient::HandleDiagnostic(
	clang::Diagnostic::Level DiagLevel,
	const clang::DiagnosticInfo &Info)
{
	if (!_DC || _hadErrors) return;

	if (Info.getID() != clang::diag::warn_unused_expr &&
	    Info.getID() != clang::diag::pp_macro_not_used)
		_DC->HandleDiagnostic(DiagLevel, Info);
	if (DiagLevel == clang::Diagnostic::Error)
		_hadErrors = true;
}

bool ProxyDiagnosticClient::hadErrors() const
{
	return _hadErrors;
}

StmtFinder::StmtFinder(unsigned pos, const clang::SourceManager& sm)
	: _pos(pos), _sm(sm), _S(NULL)
{
}

StmtFinder::~StmtFinder()
{
}

void StmtFinder::VisitChildren(clang::Stmt *S) {
	for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
	     I != E; ++I) {
		if (*I) {
			Visit(*I);
		}
	}
}

void StmtFinder::VisitStmt(clang::Stmt *S) {
	clang::SourceLocation Loc = S->getLocStart();
	if (_sm.getFileOffset(_sm.getInstantiationLoc(Loc)) == _pos) {
		_S = S;
	}
}

clang::Stmt * StmtFinder::getStmt() const
{
	return _S;
}


FunctionBodyConsumer::FunctionBodyConsumer(StmtFinder *SF)
	: _SF(SF), _seenFunction(false)
{
}

FunctionBodyConsumer::~FunctionBodyConsumer()
{
}

void FunctionBodyConsumer::HandleTopLevelDecl(clang::Decl *D) {
	if (clang::FunctionDecl *FD = dyn_cast<clang::FunctionDecl>(D)) {
		if (clang::Stmt *S = FD->getBody()) {
			_SF->VisitChildren(S);
			_seenFunction = true;
		}
	}
}

bool FunctionBodyConsumer::seenFunction() const
{
	return _seenFunction;
}


} // namespace ccons
