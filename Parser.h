#ifndef CCONS_PARSER_H
#define CCONS_PARSER_H

#include <string>

#include <llvm/ADT/OwningPtr.h>

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/FileManager.h>

namespace clang {
	class ASTConsumer;
	class Diagnostic;
	class Preprocessor;
} // namespace clang


namespace ccons {

class Parser {

public:

	explicit Parser(const clang::LangOptions& options);
	virtual ~Parser();

	clang::Preprocessor * parse(std::string source, clang::Diagnostic *diag, clang::ASTConsumer *consumer);

private:

	const clang::LangOptions& _options;
	llvm::OwningPtr<clang::TargetInfo> _target;
	clang::FileManager _fm;

};

} // namespace ccons

#endif // CCONS_PARSER_H
