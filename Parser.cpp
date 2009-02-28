#include "Parser.h"

#include <iostream>
#include <stack>
#include <algorithm>

#include <llvm/Config/config.h>
#include <llvm/Support/MemoryBuffer.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/TranslationUnit.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Driver/InitHeaderSearch.h>
#include <clang/Driver/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Sema/ParseAST.h>

#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/Diagnostic.h>


#include "ClangUtils.h"
#include "SrcGen.h"

using std::string;

namespace ccons {

Parser::Parser(const clang::LangOptions& options) :
	_options(options),
	_target(clang::TargetInfo::CreateTargetInfo(LLVM_HOSTTRIPLE))
{
}

clang::ASTContext * Parser::getContext() const
{
	return _ast.get();
}

Parser::InputType Parser::analyzeInput(const string& contextSource,
                                       const string& buffer,
                                       int& indentLevel,
                                       const clang::FunctionDecl*& FD)
{
	clang::SourceManager sm;
	createMemoryBuffer(buffer, "", &sm);
	clang::FileManager fm;
	clang::HeaderSearch headers(fm);
	clang::InitHeaderSearch ihs(headers);
	ihs.AddDefaultEnvVarPaths(_options);
	ihs.AddDefaultSystemIncludePaths(_options);
	ihs.Realize();
	ProxyDiagnosticClient pdc(NULL);
	clang::Diagnostic diag(&pdc);
	clang::Preprocessor PP(diag, _options, *_target, sm, headers);

	std::stack<std::pair<clang::Token, clang::Token> > S; // Tok, PrevTok

	indentLevel = 0;
	PP.EnterMainSourceFile();

	clang::Token Tok, LastTok;
	bool TokWasDo = false;
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
				return Incomplete;
			}
			TokWasDo = false;
			S.pop();
		} else if (Tok.is(clang::tok::r_paren)) {
			if (S.empty() || S.top().first.isNot(clang::tok::l_paren)) {
				std::cout << "Unmatched (\n";
				return Incomplete;
			}
			TokWasDo = false;
			S.pop();
		} else if (Tok.is(clang::tok::r_brace)) {
			if (S.empty() || S.top().first.isNot(clang::tok::l_brace)) {
				std::cout << "Unmatched {\n";
				return Incomplete;
			}
			TokWasDo = S.top().second.is(clang::tok::kw_do);
			S.pop();
			indentLevel--;
		}
		LastTok = Tok;
		PP.Lex(Tok);
	}

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

	// TokWasDo is used for do { ... } while (...); loops
	if (LastTok.is(clang::tok::semi) || (LastTok.is(clang::tok::r_brace) && !TokWasDo)) {
		if (!S.empty()) return Incomplete;
		// FIXME: send diagnostics to /dev/null
		clang::TextDiagnosticPrinter tdp(llvm::errs(), false, true, false);
		ProxyDiagnosticClient pdc(NULL);
		clang::Diagnostic diag(&pdc);
		diag.setSuppressSystemWarnings(true);
		string src = contextSource + buffer;
		clang::SourceManager sm;
		struct : public clang::ASTConsumer {
			unsigned pos;
			unsigned maxPos;
			clang::SourceManager *sm;
			clang::FunctionDecl *FD;
			void HandleTopLevelDecl(clang::Decl *D) {
				if (clang::FunctionDecl *FuD = dyn_cast<clang::FunctionDecl>(D)) {
					clang::SourceLocation Loc = FuD->getTypeSpecStartLoc();
					unsigned offset = sm->getFileOffset(sm->getInstantiationLoc(Loc));
					if (offset == pos) {
						this->FD = FuD;
					}
				}
			}
		} consumer;
		consumer.pos = contextSource.length();
		consumer.maxPos = consumer.pos + buffer.length();
		consumer.sm = &sm;
		consumer.FD = NULL;
		parse(src, &sm, &diag, &consumer);
		if (!pdc.hadErrors() && consumer.FD) {
			FD = consumer.FD;
			return TopLevel;
		}
		return Stmt;
	}

	return Incomplete;
}

void Parser::parse(const string& src,
                   clang::SourceManager *sm,
                   clang::Diagnostic *diag,
                   clang::ASTConsumer *consumer)
{
	createMemoryBuffer(src, "Main", sm);
	clang::HeaderSearch headers(_fm);
	clang::InitHeaderSearch ihs(headers);
	ihs.AddDefaultEnvVarPaths(_options);
	ihs.AddDefaultSystemIncludePaths(_options);
	ihs.Realize();

	_pp.reset(new clang::Preprocessor(*diag, _options, *_target, *sm, headers));
	_ast.reset(new clang::ASTContext(_options, *sm, *_target,
		_pp->getIdentifierTable(), _pp->getSelectorTable()));
	_tu.reset(new clang::TranslationUnit(*_ast));

	clang::ParseAST(*_pp, consumer, _tu.get());
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
