#include "Visitors.h"

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Sema/SemaDiagnostic.h>

using std::string;

namespace ccons {

//
// StmtFinder
//

StmtFinder::StmtFinder(unsigned pos, const clang::SourceManager& sm)
	: _pos(pos), _sm(sm), _S(NULL)
{
}

StmtFinder::~StmtFinder()
{
}

void StmtFinder::VisitChildren(clang::Stmt *S)
{
	for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
	     I != E; ++I) {
		if (*I) {
			Visit(*I);
		}
	}
}

void StmtFinder::VisitStmt(clang::Stmt *S)
{
	clang::SourceLocation Loc = S->getLocStart();
	unsigned offs = _sm.getFileOffset(_sm.getInstantiationLoc(Loc));
	if (offs == _pos) {
		_S = S;
	}
}

clang::Stmt * StmtFinder::getStmt() const
{
	return _S;
}

//
// StmtSplitter
//

StmtSplitter::StmtSplitter(const string& src,
                           const clang::SourceManager& sm,
                           const clang::LangOptions& options,
                           std::vector<clang::Stmt*> *stmts)
		: _src(src), _sm(sm), _options(options), _stmts(stmts)
{
}

StmtSplitter::~StmtSplitter()
{
}

void StmtSplitter::VisitChildren(clang::Stmt *S)
{
	for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
	     I != E; ++I) {
		if (*I) {
			Visit(*I);
		}
	}
}

void StmtSplitter::VisitStmt(clang::Stmt *S)
{
	_stmts->push_back(S);
}

} // namespace ccons
