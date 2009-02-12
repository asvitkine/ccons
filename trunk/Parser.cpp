#include "Parser.h"

#include <iostream>
#include <stack>

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

bool Parser::analyzeInput(const string& buffer, int& indentLevel)
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

	std::stack<clang::Token> S;

	indentLevel = 0;
	PP.EnterMainSourceFile();

	clang::Token Tok, LastTok;
	PP.Lex(Tok);
	while (Tok.isNot(clang::tok::eof)) {
		LastTok = Tok;
		if (Tok.is(clang::tok::l_square)) {
			S.push(Tok); // [
		} else if (Tok.is(clang::tok::l_paren)) {
			S.push(Tok); // (
		} else if (Tok.is(clang::tok::l_brace)) {
			S.push(Tok); // {
			indentLevel++;
		} else if (Tok.is(clang::tok::r_square)) {
			if (S.empty() || S.top().isNot(clang::tok::l_square)) {
				std::cout << "Unmatched [\n";
				return false;
			}
			S.pop();
		} else if (Tok.is(clang::tok::r_paren)) {
			if (S.empty() || S.top().isNot(clang::tok::l_paren)) {
				std::cout << "Unmatched (\n";
				return false;
			}
			S.pop();
		} else if (Tok.is(clang::tok::r_brace)) {
			if (S.empty() || S.top().isNot(clang::tok::l_brace)) {
				std::cout << "Unmatched {\n";
				return false;
			}
			S.pop();
			indentLevel--;
		}
		PP.Lex(Tok);
	}

	// TODO: We need to properly account for indent-level for blocks that do not
	//       have braces... such as:
	//
	//       if (X)
	//         Y;

	if (LastTok.is(clang::tok::semi) || LastTok.is(clang::tok::r_brace))
		return S.empty();

	return false;
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
