#include "ClangUtils.h"

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

} // namespace ccons
