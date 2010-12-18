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

	void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
	                      const clang::DiagnosticInfo &Info);

	void BeginSourceFile(const clang::LangOptions& opts, const clang::Preprocessor *pp);

	void setOffset(unsigned offset);

	clang::Diagnostic * getDiagnostic();

private:

	unsigned _offs;
	clang::DiagnosticOptions _dop;
	clang::TextDiagnosticPrinter _tdp;
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> _diagIDs;
	clang::Diagnostic _diag;
	std::set<std::pair<clang::diag::kind, unsigned> > _memory;

};


//
// ProxyDiagnosticClient can act as a proxy to another diagnostic client.
//

class ProxyDiagnosticClient : public clang::DiagnosticClient {

public:

	explicit ProxyDiagnosticClient(clang::DiagnosticClient *DC);

	void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
	                      const clang::DiagnosticInfo &Info);

	bool hadError(clang::diag::kind Kind) const;
	bool hadErrors() const;

private:

	clang::DiagnosticClient *_DC;
	std::multimap<clang::diag::kind, const clang::DiagnosticInfo> _errors;

};

//
// NullDiagnosticProvider
//

class NullDiagnosticProvider {

public:

	NullDiagnosticProvider();

	clang::Diagnostic * getDiagnostic();
	ProxyDiagnosticClient * getProxyDiagnosticClient();

private:

	ProxyDiagnosticClient *_pdc; // owner by _diagnostic
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> _diagnosticIDs;
	clang::Diagnostic _diagnostic;

};


} // namespace ccons

#endif // CCONS_DIAGNOSTICS_H
