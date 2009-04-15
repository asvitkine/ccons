#include "Diagnostics.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/LexDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

namespace ccons {

DiagnosticsProvider::DiagnosticsProvider(llvm::raw_os_ostream& out)
	: _tdp(out, false, true, false)
	, _diag(this)
{
	_diag.setDiagnosticMapping(clang::diag::ext_implicit_function_decl,
	                           clang::diag::MAP_ERROR);
	_diag.setDiagnosticMapping(clang::diag::warn_unused_expr,
	                           clang::diag::MAP_IGNORE);
	_diag.setDiagnosticMapping(clang::diag::warn_missing_prototype,
	                           clang::diag::MAP_IGNORE);
	_diag.setDiagnosticMapping(clang::diag::pp_macro_not_used,
	                           clang::diag::MAP_IGNORE);
	_diag.setSuppressSystemWarnings(true);
}

void DiagnosticsProvider::HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
                                           const clang::DiagnosticInfo &Info)
{
	std::pair<clang::diag::kind, unsigned> record = std::make_pair(Info.getID(),
		Info.getLocation().getManager().getFileOffset(Info.getLocation()) - _offs);
	if (_memory.insert(record).second) {
		_tdp.HandleDiagnostic(DiagLevel, Info);
	}
}

void DiagnosticsProvider::setOffset(unsigned offset)
{
	_offs = offset;
}

clang::Diagnostic * DiagnosticsProvider::getDiagnostic()
{
	return &_diag;
}

} // namespace ccons

