#include "Parser.h"

#include <llvm/Config/config.h>
#include <llvm/Support/MemoryBuffer.h>

#include <clang/AST/ASTConsumer.h>
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

Parser::~Parser()
{
}

clang::Preprocessor * Parser::parse(string source, clang::Diagnostic *diag, clang::ASTConsumer *consumer)
{
	llvm::MemoryBuffer *mb = llvm::MemoryBuffer::getMemBufferCopy(&*source.begin(), &*source.end(), "Main");
	assert(mb);
	clang::SourceManager sm;
	sm.createMainFileIDForMemBuffer(mb);
	assert(sm.getMainFileID());
	clang::HeaderSearch headers(_fm);
	clang::InitHeaderSearch ihs(headers);
	ihs.AddDefaultEnvVarPaths(_options);
	ihs.AddDefaultSystemIncludePaths(_options);
	ihs.Realize();
	clang::Preprocessor *pp = new clang::Preprocessor(*diag, _options, *_target, sm, headers);
	clang::ParseAST(*pp, consumer, /* PrintStats = */ false, /* FreeMemory = */ false);
	return pp;
}

} // namespace ccons
