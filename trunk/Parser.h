#ifndef CCONS_PARSER_H
#define CCONS_PARSER_H

#include <string>
#include <vector>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/MemoryBuffer.h>

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Lex/HeaderSearch.h>

namespace clang {
	class ASTConsumer;
	class ASTContext;
	class Diagnostic;
	class FunctionDecl;
	class Preprocessor;
	class SourceManager;
	class TargetInfo;
	class Token;
} // namespace clang


namespace ccons {

class ParseOperation {

public:
	
	ParseOperation(const clang::LangOptions& options,
	               clang::TargetInfo& target,
	               clang::Diagnostic *diag,
	               clang::SourceManager *sm = 0);

  clang::ASTContext * getASTContext() const;
	clang::Preprocessor * getPreprocessor() const;
	clang::SourceManager * getSourceManager() const;

private:

	llvm::OwningPtr<clang::SourceManager> _sm;
	llvm::OwningPtr<clang::FileManager> _fm;
	llvm::OwningPtr<clang::HeaderSearch> _hs;
	llvm::OwningPtr<clang::Preprocessor> _pp;
	llvm::OwningPtr<clang::ASTContext> _ast;

};

class Parser {

public:

	explicit Parser(const clang::LangOptions& options);
	~Parser();

	enum InputType { Incomplete, TopLevel, Stmt }; 

	InputType analyzeInput(const std::string& contextSource,
	                       const std::string& buffer,
	                       int& indentLevel,
	                       const clang::FunctionDecl*& FD);
	void parse(const std::string& source,
	           clang::Diagnostic *diag,
	           clang::ASTConsumer *consumer,
	           clang::SourceManager *sm = 0);

  ParseOperation * getLastParseOperation() const;
	void releaseAccumulatedParseOperations();

private:

	const clang::LangOptions& _options;
	llvm::OwningPtr<clang::TargetInfo> _target;
	std::vector<ParseOperation*> _ops;

	unsigned analyzeTokens(clang::Preprocessor& PP,
	                       clang::Token& LastTok,
	                       int& indentLevel,
	                       bool& TokWasDo);

	static llvm::MemoryBuffer * createMemoryBuffer(const std::string& src,
	                                               const char *name,
	                                               clang::SourceManager *sm);

};

} // namespace ccons

#endif // CCONS_PARSER_H
