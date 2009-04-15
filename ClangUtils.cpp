#include "ClangUtils.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Sema/SemaDiagnostic.h>

namespace ccons {

//
// ProxyDiagnosticClient
//

ProxyDiagnosticClient::ProxyDiagnosticClient(clang::DiagnosticClient *DC)
	: _DC(DC), _hadErrors(false)
{
}

void ProxyDiagnosticClient::HandleDiagnostic(
	clang::Diagnostic::Level DiagLevel,
	const clang::DiagnosticInfo &Info)
{
	if (DiagLevel == clang::Diagnostic::Error)
		_hadErrors = true;

	if (!_DC || _hadErrors) return;

	_DC->HandleDiagnostic(DiagLevel, Info);
}

bool ProxyDiagnosticClient::hadErrors() const
{
	return _hadErrors;
}

//
// getStmtRange
//

SrcRange getStmtRange(const clang::Stmt *S,
                      const clang::SourceManager& sm,
                      const clang::LangOptions options)
{
	clang::SourceLocation SLoc = sm.getInstantiationLoc(S->getLocStart());
	clang::SourceLocation ELoc = sm.getInstantiationLoc(S->getLocEnd());
	unsigned start = sm.getFileOffset(SLoc);
	unsigned end   = sm.getFileOffset(ELoc);
	end += clang::Lexer::MeasureTokenLength(ELoc, sm, options);
	return SrcRange(start, end);
}

} // namespace ccons
