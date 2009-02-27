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
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Sema/ParseAST.h>

#include "ClangUtils.h"

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
                                       int& indentLevel)
{
	llvm::MemoryBuffer *mb =
		llvm::MemoryBuffer::getMemBufferCopy(&*buffer.begin(), &*buffer.end(), "");
	assert(mb && "Error creating MemoryBuffer!");
	clang::SourceManager sm;
	sm.createMainFileIDForMemBuffer(mb);
	assert(!sm.getMainFileID().isInvalid() && "Error creating MainFileID!");

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

	// TokWasDo is used for do { ... } while (...); loops
	if (LastTok.is(clang::tok::semi) || (LastTok.is(clang::tok::r_brace) && !TokWasDo)) {
		if (!S.empty()) return Incomplete;
		return Stmt;
	}

	return Incomplete;
}

void Parser::parse(const string& src,
                   clang::SourceManager *sm,
                   clang::Diagnostic *diag,
                   clang::ASTConsumer *consumer)
{
	llvm::MemoryBuffer *mb =
		llvm::MemoryBuffer::getMemBufferCopy(&*src.begin(), &*src.end(), "Main");
	assert(mb && "Error creating MemoryBuffer!");
	sm->createMainFileIDForMemBuffer(mb);
	assert(!sm->getMainFileID().isInvalid() && "Error creating MainFileID!");

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

} // namespace ccons
