//
// Utilities related to diagnostics reporting for ccons.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "Diagnostics.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/LexDiagnostic.h>
#include <clang/Sema/SemaDiagnostic.h>

namespace ccons {

//
// DiagnosticsProvider
//

DiagnosticsProvider::DiagnosticsProvider(llvm::raw_os_ostream& out)
	: _tdp(out, _dop)
	, _diag(this)
{
	_dop.ShowColumn = 0;
	_dop.ShowSourceRanges = 1;
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

void DiagnosticsProvider::BeginSourceFile(const clang::LangOptions& opts, const clang::Preprocessor *pp)
{
	_tdp.BeginSourceFile(opts, pp);
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

//
// ProxyDiagnosticClient
//

ProxyDiagnosticClient::ProxyDiagnosticClient(clang::DiagnosticClient *DC)
	: _DC(DC)
{
}

void ProxyDiagnosticClient::HandleDiagnostic(
	clang::Diagnostic::Level DiagLevel,
	const clang::DiagnosticInfo &Info)
{
	if (DiagLevel == clang::Diagnostic::Error)
		_errors.insert(std::make_pair(Info.getID(), Info));

	if (_DC)
		_DC->HandleDiagnostic(DiagLevel, Info);
}

bool ProxyDiagnosticClient::hadError(clang::diag::kind Kind) const
{
	return _errors.find(Kind) != _errors.end();
}

bool ProxyDiagnosticClient::hadErrors() const
{
	return !_errors.empty();
}

} // namespace ccons

