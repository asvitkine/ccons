#ifndef CCONS_PARSER_H
#define CCONS_PARSER_H

//
// Parser is used to invoke the clang libraries to perform actual parsing of
// the input received in the Console.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

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
	               clang::Diagnostic *diag);

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
	                       std::vector<clang::FunctionDecl*> *fds);

	// Create a new ParseOperation that the caller should take ownership of
	// and the lifetime of which must be shorter than of the Parser.
	ParseOperation * createParseOperation(clang::Diagnostic *diag);

	// Parse the specified source code with the specified parse operation
	// and consumer. Upon parsing, ownership of parseOp is transferred to
	// the Parser permanently.
	void parse(const std::string& src,
	           ParseOperation *parseOp,
						 clang::ASTConsumer *consumer);

	// Parse by implicitely creating a ParseOperation. Equivalent to
	// parse(src, createParseOperation(diag), consumer).
	void parse(const std::string& src,
	           clang::Diagnostic *diag,
						 clang::ASTConsumer *consumer);

  ParseOperation * getLastParseOperation() const;
	void releaseAccumulatedParseOperations();

private:

	const clang::LangOptions& _options;
	llvm::OwningPtr<clang::TargetInfo> _target;
	std::vector<ParseOperation*> _ops;

	int analyzeTokens(clang::Preprocessor& PP,
	                  clang::Token& LastTok,
	                  int& indentLevel,
	                  bool& TokWasDo);

	static llvm::MemoryBuffer * createMemoryBuffer(const std::string& src,
	                                               const char *name,
	                                               clang::SourceManager *sm);

};

} // namespace ccons

#endif // CCONS_PARSER_H
