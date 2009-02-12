#ifndef CCONS_CLANG_UTILS_H
#define CCONS_CLANG_UTILS_H

#include <clang/Basic/Diagnostic.h>

namespace ccons {

class ProxyDiagnosticClient : public clang::DiagnosticClient {

public:

	ProxyDiagnosticClient(clang::DiagnosticClient *DC);

	void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
	                      const clang::DiagnosticInfo &Info);

	bool hadErrors() const;

private:

	clang::DiagnosticClient *_DC;
	bool _hadErrors;

};

} // namespace ccons

#endif // CCONS_CLANG_UTILS_H
