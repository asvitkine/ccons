#include "Parser.h"

#include <llvm/Config/config.h>
#include <llvm/Support/MemoryBuffer.h>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/TranslationUnit.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Driver/InitHeaderSearch.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Sema/ParseAST.h>

using std::string;

namespace ccons {

Parser::Parser(const clang::LangOptions& options) :
	_options(options),
	_target(clang::TargetInfo::CreateTargetInfo(LLVM_HOSTTRIPLE))
{
}

void Parser::parse(string src,
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
