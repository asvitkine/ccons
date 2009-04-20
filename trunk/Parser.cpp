//
// Parser is used to invoke the clang libraries to perform actual parsing of
// the input received in the Console.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include "Parser.h"

#include <iostream>
#include <stack>
#include <algorithm>

#include <llvm/Config/config.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/InitHeaderSearch.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Sema/ParseAST.h>
#include <clang/Sema/SemaDiagnostic.h>

#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/Diagnostic.h>

#include "ClangUtils.h"
#include "SrcGen.h"

// Temporary Hax:
#include "InitPP.cpp"

using std::string;

namespace ccons {


//
// ParseOperation
//

ParseOperation::ParseOperation(const clang::LangOptions& options,
                               clang::TargetInfo& target,
                               clang::Diagnostic *diag) :
	_sm(new clang::SourceManager),
	_fm(new clang::FileManager),
	_hs(new clang::HeaderSearch(*_fm))
{
	clang::InitHeaderSearch ihs(*_hs);
	ihs.AddDefaultEnvVarPaths(options);
	ihs.AddDefaultSystemIncludePaths(options);
	ihs.Realize();
	_pp.reset(new clang::Preprocessor(*diag, options, target, *_sm, *_hs));
	InitializePreprocessor(*_pp);
	_ast.reset(new clang::ASTContext(options, *_sm, target,
		_pp->getIdentifierTable(), _pp->getSelectorTable()));
}

clang::ASTContext * ParseOperation::getASTContext() const
{
	return _ast.get();
}

clang::Preprocessor * ParseOperation::getPreprocessor() const
{
	return _pp.get();
}

clang::SourceManager * ParseOperation::getSourceManager() const
{
	return _sm.get();
}


//
// Parser
//

Parser::Parser(const clang::LangOptions& options) :
	_options(options),
	_target(clang::TargetInfo::CreateTargetInfo(LLVM_HOSTTRIPLE))
{
}

Parser::~Parser()
{
	releaseAccumulatedParseOperations();
}

void Parser::releaseAccumulatedParseOperations()
{
	for (std::vector<ParseOperation*>::iterator I = _ops.begin(), E = _ops.end();
	     I != E; ++I) {
		delete *I;
	}
	_ops.clear();
}


ParseOperation * Parser::getLastParseOperation() const
{
	return _ops.back();
}

Parser::InputType Parser::analyzeInput(const string& contextSource,
                                       const string& buffer,
                                       int& indentLevel,
                                       std::vector<clang::FunctionDecl*> *fds)
{
	if (buffer.length() > 1 && buffer[buffer.length() - 2] == '\\') {
		indentLevel = 1;
		return Incomplete;
	}
	
	ProxyDiagnosticClient pdc(NULL);
	clang::Diagnostic diag(&pdc);
	llvm::OwningPtr<ParseOperation>
		parseOp(new ParseOperation(_options, *_target, &diag));
	createMemoryBuffer(buffer, "", parseOp->getSourceManager());

	clang::Token LastTok;
	bool TokWasDo = false;
	int stackSize =
		analyzeTokens(*parseOp->getPreprocessor(), LastTok, indentLevel, TokWasDo);
	if (stackSize < 0)
		return Incomplete;

	// TokWasDo is used for do { ... } while (...); loops
	if (LastTok.is(clang::tok::semi) || (LastTok.is(clang::tok::r_brace) && !TokWasDo)) {
		if (stackSize > 0) return Incomplete;
		ProxyDiagnosticClient pdc(NULL); // do not output diagnostics
		clang::Diagnostic diag(&pdc);
		// Setting this ensures "foo();" is not a valid top-level declaration.
		diag.setDiagnosticMapping(clang::diag::warn_missing_type_specifier,
	                            clang::diag::MAP_ERROR);
		diag.setSuppressSystemWarnings(true);
		string src = contextSource + buffer;
		struct : public clang::ASTConsumer {
			unsigned pos;
			unsigned maxPos;
			clang::SourceManager *sm;
			std::vector<clang::FunctionDecl*> fds;
			void HandleTopLevelDecl(clang::DeclGroupRef D) {
				for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
					if (clang::FunctionDecl *FD = dyn_cast<clang::FunctionDecl>(*I)) {
						clang::SourceLocation Loc = FD->getTypeSpecStartLoc();
						if (sm->getFileID(Loc) == sm->getMainFileID()) {
							unsigned offset = sm->getFileOffset(sm->getInstantiationLoc(Loc));
							if (offset >= pos) {
								fds.push_back(FD);
							}
						}
					}
				}
			}
		} consumer;
		ParseOperation *parseOp = createParseOperation(&diag);
		consumer.pos = contextSource.length();
		consumer.maxPos = consumer.pos + buffer.length();
		consumer.sm = parseOp->getSourceManager();
		parse(src, parseOp, &consumer);
		if (!pdc.hadErrors() && !consumer.fds.empty()) {
			fds->swap(consumer.fds);
			return TopLevel;
		}
		return Stmt;
	}

	return Incomplete;
}

int Parser::analyzeTokens(clang::Preprocessor& PP,
                          clang::Token& LastTok,
                          int& indentLevel,
                          bool& TokWasDo)
{
	int result;
	std::stack<std::pair<clang::Token, clang::Token> > S; // Tok, PrevTok

	indentLevel = 0;
	PP.EnterMainSourceFile();

	clang::Token Tok;
	PP.Lex(Tok);
	while (Tok.isNot(clang::tok::eof)) {
		if (Tok.is(clang::tok::l_square)) {
			S.push(std::make_pair(Tok, LastTok)); // [
		} else if (Tok.is(clang::tok::l_paren)) {
			S.push(std::make_pair(Tok, LastTok)); // (
		} else if (Tok.is(clang::tok::l_brace)) {
			S.push(std::make_pair(Tok, LastTok)); // {
			indentLevel++;
		} else if (Tok.is(clang::tok::r_square)) {
			if (S.empty() || S.top().first.isNot(clang::tok::l_square)) {
				std::cout << "Unmatched [\n";
				return -1;
			}
			TokWasDo = false;
			S.pop();
		} else if (Tok.is(clang::tok::r_paren)) {
			if (S.empty() || S.top().first.isNot(clang::tok::l_paren)) {
				std::cout << "Unmatched (\n";
				return -1;
			}
			TokWasDo = false;
			S.pop();
		} else if (Tok.is(clang::tok::r_brace)) {
			if (S.empty() || S.top().first.isNot(clang::tok::l_brace)) {
				std::cout << "Unmatched {\n";
				return -1;
			}
			TokWasDo = S.top().second.is(clang::tok::kw_do);
			S.pop();
			indentLevel--;
		}
		LastTok = Tok;
		PP.Lex(Tok);
	}
	result = S.size();

	// TODO: We need to properly account for indent-level for blocks that do not
	//       have braces... such as:
	//
	//       if (X)
	//         Y;
	//
	// TODO: Do-while without braces doesn't work, e.g.:
	//
	//       do
	//         foo();
	//       while (bar());
	//
	// Both of the above could be solved by some kind of rewriter-pass that would
	// insert implicit braces (or simply a more involved analysis).

	// Also try to match preprocessor conditionals...
	if (result == 0) {
		clang::Lexer Lexer(PP.getSourceManager().getMainFileID(),
		                   PP.getSourceManager(),
		                   PP.getLangOptions());
		Lexer.LexFromRawLexer(Tok);
		while (Tok.isNot(clang::tok::eof)) {
			if (Tok.is(clang::tok::hash)) {
				Lexer.LexFromRawLexer(Tok);
				if (clang::IdentifierInfo *II = PP.LookUpIdentifierInfo(Tok)) { 
					switch (II->getPPKeywordID()) {
						case clang::tok::pp_if:
						case clang::tok::pp_ifdef:
						case clang::tok::pp_ifndef:
							result++;
							break;
						case clang::tok::pp_endif:
							if (result == 0)
								return -1; // Nesting error.
							result--;
							break;
						default:
							break;
					}
				}
			}
			Lexer.LexFromRawLexer(Tok);
		}
	}

	return result;
}

ParseOperation * Parser::createParseOperation(clang::Diagnostic *diag)
{
	return new ParseOperation(_options, *_target, diag);
}

void Parser::parse(const string& src,
                   ParseOperation *parseOp,
                   clang::ASTConsumer *consumer)
{
	_ops.push_back(parseOp);
	createMemoryBuffer(src, "", parseOp->getSourceManager());
	clang::ParseAST(*parseOp->getPreprocessor(), consumer,
	                *parseOp->getASTContext());
}


void Parser::parse(const string& src,
                   clang::Diagnostic *diag,
                   clang::ASTConsumer *consumer)
{
	parse(src, createParseOperation(diag), consumer);
}


llvm::MemoryBuffer * Parser::createMemoryBuffer(const string& src,
                                                const char *name,
                                                clang::SourceManager *sm)
{
	llvm::MemoryBuffer *mb =
		llvm::MemoryBuffer::getMemBufferCopy(&*src.begin(), &*src.end(), name);
	assert(mb && "Error creating MemoryBuffer!");
	sm->createMainFileIDForMemBuffer(mb);
	assert(!sm->getMainFileID().isInvalid() && "Error creating MainFileID!");
	return mb;
}

} // namespace ccons
