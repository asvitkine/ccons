#ifndef CCONS_DIAGNOSTICS_H
#define CCONS_DIAGNOSTICS_H

#include <llvm/Support/raw_ostream.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

namespace ccons {

class DiagnosticsProvider {

public:

	explicit DiagnosticsProvider(llvm::raw_os_ostream& out);

	clang::Diagnostic * getDiagnostic();

private:

	clang::TextDiagnosticPrinter _tdp;
	clang::Diagnostic _diag;

};

} // namespace ccons

#endif // CCONS_DIAGNOSTICS_H
