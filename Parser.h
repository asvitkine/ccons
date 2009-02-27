#ifndef CCONS_PARSER_H
#define CCONS_PARSER_H

#include <string>

#include <llvm/ADT/OwningPtr.h>

#include <clang/AST/TranslationUnit.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/FileManager.h>

namespace clang {
	class ASTConsumer;
	class ASTContext;
	class Diagnostic;
	class Preprocessor;
	class SourceManager;
	class TargetInfo;
	class TranslationUnit;
} // namespace clang


namespace ccons {

class Parser {

public:

	explicit Parser(const clang::LangOptions& options);

	enum InputType { Incomplete, TopLevel, Stmt}; 

	InputType analyzeInput(const std::string& contextSource,
	                       const std::string& buffer,
	                       int& indentLevel);
	void parse(const std::string& source,
						 clang::SourceManager *sm,
	           clang::Diagnostic *diag,
	           clang::ASTConsumer *consumer);

  clang::ASTContext * getContext() const;

private:

	const clang::LangOptions& _options;
	clang::FileManager _fm;
	llvm::OwningPtr<clang::TargetInfo> _target;
	llvm::OwningPtr<clang::Preprocessor> _pp;
	llvm::OwningPtr<clang::TranslationUnit> _tu;
	llvm::OwningPtr<clang::ASTContext> _ast;

};

} // namespace ccons

#endif // CCONS_PARSER_H
