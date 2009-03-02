#include "Diagnostics.h"

#include <clang/Sema/SemaDiagnostic.h>

namespace ccons {

DiagnosticsProvider::DiagnosticsProvider(llvm::raw_os_ostream& out)
	: _tdp(out, false, true, false)
	, _diag(&_tdp)
{
	_diag.setDiagnosticMapping(clang::diag::ext_implicit_function_decl,
	                           clang::diag::MAP_ERROR);
	_diag.setDiagnosticMapping(clang::diag::warn_unused_expr,
	                           clang::diag::MAP_IGNORE);
	_diag.setDiagnosticMapping(clang::diag::pp_macro_not_used,
	                           clang::diag::MAP_IGNORE);
	_diag.setSuppressSystemWarnings(true);
}

clang::Diagnostic * DiagnosticsProvider::getDiagnostic()
{
	return &_diag;
}

} // namespace ccons

