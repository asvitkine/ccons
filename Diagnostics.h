#ifndef CCONS_DIAGNOSTICS_H
#define CCONS_DIAGNOSTICS_H

#include <set>

#include <llvm/Support/raw_ostream.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

namespace ccons {

class DiagnosticsProvider : public clang::DiagnosticClient {

public:

	explicit DiagnosticsProvider(llvm::raw_os_ostream& out);

	void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
	                      const clang::DiagnosticInfo &Info);

	void setOffset(unsigned offset);

	clang::Diagnostic * getDiagnostic();

private:

	unsigned _offs;
	clang::TextDiagnosticPrinter _tdp;
	clang::Diagnostic _diag;
	std::set<std::pair<clang::diag::kind, unsigned> > _memory;

};

} // namespace ccons

#endif // CCONS_DIAGNOSTICS_H
