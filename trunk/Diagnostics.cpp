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

namespace {
	class DiagnosticsProviderConsumer : public clang::DiagnosticConsumer {
	public:

		DiagnosticsProviderConsumer(DiagnosticsProvider *dp)
			: _dp(dp)
		{
		}

		void BeginSourceFile(const clang::LangOptions& opts,
		                     const clang::Preprocessor *pp)
		{
			_dp->BeginSourceFile(opts, pp);
		}

		void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
		                      const clang::Diagnostic& Info)
		{
			_dp->HandleDiagnostic(DiagLevel, Info);
		}

		clang::DiagnosticConsumer *clone(clang::DiagnosticsEngine& engine) const
		{
			return new DiagnosticsProviderConsumer(_dp);
		}

	private:
		DiagnosticsProvider *_dp;
	};
}

DiagnosticsProvider::DiagnosticsProvider(llvm::raw_os_ostream& out)
	: _tdp(out, _dop)
	, _diagIDs(new clang::DiagnosticIDs())
	, _engine(_diagIDs, new DiagnosticsProviderConsumer(this))
{
	_dop.ShowColumn = 0;
	_dop.ShowSourceRanges = 1;
	_engine.setDiagnosticMapping((clang::diag::kind)clang::diag::ext_implicit_function_decl,
	                             clang::diag::MAP_ERROR, clang::SourceLocation());
	_engine.setDiagnosticMapping((clang::diag::kind)clang::diag::warn_unused_expr,
	                             clang::diag::MAP_IGNORE, clang::SourceLocation());
	_engine.setDiagnosticMapping((clang::diag::kind)clang::diag::warn_missing_prototype,
	                             clang::diag::MAP_IGNORE, clang::SourceLocation());
	_engine.setDiagnosticMapping((clang::diag::kind)clang::diag::pp_macro_not_used,
	                             clang::diag::MAP_IGNORE, clang::SourceLocation());
	_engine.setSuppressSystemWarnings(true);
}

void DiagnosticsProvider::BeginSourceFile(const clang::LangOptions& opts, const clang::Preprocessor *pp)
{
	_tdp.BeginSourceFile(opts, pp);
}

void DiagnosticsProvider::HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
                                           const clang::Diagnostic& Info)
{
	std::pair<clang::diag::kind, unsigned> record = std::make_pair(Info.getID(),
		Info.getSourceManager().getFileOffset(Info.getLocation()) - _offs);
	if (_memory.insert(record).second) {
		_tdp.HandleDiagnostic(DiagLevel, Info);
	}
}

void DiagnosticsProvider::setOffset(unsigned offset)
{
	_offs = offset;
}

clang::DiagnosticsEngine * DiagnosticsProvider::getDiagnosticsEngine()
{
	return &_engine;
}

//
// ProxyDiagnosticConsumer
//

ProxyDiagnosticConsumer::ProxyDiagnosticConsumer(clang::DiagnosticConsumer *DC)
	: _DC(DC)
{
}

void ProxyDiagnosticConsumer::HandleDiagnostic(
	clang::DiagnosticsEngine::Level DiagLevel,
	const clang::Diagnostic& Info)
{
	if (DiagLevel == clang::DiagnosticsEngine::Error)
		_errors.insert(std::make_pair(Info.getID(), Info));

	if (_DC)
		_DC->HandleDiagnostic(DiagLevel, Info);
}

bool ProxyDiagnosticConsumer::hadError(clang::diag::kind Kind) const
{
	return _errors.find(Kind) != _errors.end();
}

bool ProxyDiagnosticConsumer::hadErrors() const
{
	return !_errors.empty();
}

clang::DiagnosticConsumer * ProxyDiagnosticConsumer::clone(clang::DiagnosticsEngine& engine) const
{
	ProxyDiagnosticConsumer *clone = new ProxyDiagnosticConsumer(_DC);
	clone->_errors = _errors;
	return clone;
}

//
// NullDiagnosticProvider
//

NullDiagnosticProvider::NullDiagnosticProvider()
	: _diagnosticIDs(new clang::DiagnosticIDs())
	, _engine(_diagnosticIDs, _pdc = new ProxyDiagnosticConsumer(NULL))
{
}

clang::DiagnosticsEngine * NullDiagnosticProvider::getDiagnosticsEngine()
{
	return &_engine;
}

ProxyDiagnosticConsumer * NullDiagnosticProvider::getProxyDiagnosticConsumer()
{
	return _pdc;
}

} // namespace ccons

