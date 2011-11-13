#ifndef CCONS_DIAGNOSTICS_H
#define CCONS_DIAGNOSTICS_H

//
// Utilities related to diagnostics reporting for ccons.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include <set>
#include <map>

#include <llvm/Support/raw_os_ostream.h>

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

namespace ccons {

//
// DiagnosticsProvider provides a re-usable clang::Diagnostic object
// that can be used for multiple parse operations.
//

class DiagnosticsProvider {

public:

	explicit DiagnosticsProvider(llvm::raw_os_ostream& out);

	void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
	                      const clang::Diagnostic& Info);

	void BeginSourceFile(const clang::LangOptions& opts, const clang::Preprocessor *pp);

	void setOffset(unsigned offset);

	clang::DiagnosticsEngine * getDiagnosticsEngine();

private:

	unsigned _offs;
	clang::DiagnosticOptions _dop;
	clang::TextDiagnosticPrinter _tdp;
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> _diagIDs;
	clang::DiagnosticsEngine _engine;
	std::set<std::pair<clang::diag::kind, unsigned> > _memory;

};


//
// ProxyDiagnosticConsumer can act as a proxy to another diagnostic client.
//

class ProxyDiagnosticConsumer : public clang::DiagnosticConsumer {

public:

	explicit ProxyDiagnosticConsumer(clang::DiagnosticConsumer *DC);

	void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
	                      const clang::Diagnostic& Info);

	bool hadError(clang::diag::kind Kind) const;
	bool hadErrors() const;
	clang::DiagnosticConsumer * clone(clang::DiagnosticsEngine& Diags) const;

private:

	clang::DiagnosticConsumer *_DC;
	std::multimap<clang::diag::kind, const clang::Diagnostic> _errors;

};

//
// NullDiagnosticProvider
//

class NullDiagnosticProvider {

public:

	NullDiagnosticProvider();

	clang::DiagnosticsEngine * getDiagnosticsEngine();
	ProxyDiagnosticConsumer * getProxyDiagnosticConsumer();

private:

	ProxyDiagnosticConsumer *_pdc; // owned by _engine
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> _diagnosticIDs;
	clang::DiagnosticsEngine _engine;

};


} // namespace ccons

#endif // CCONS_DIAGNOSTICS_H
