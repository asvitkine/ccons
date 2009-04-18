#ifndef CCONS_VISITORS_H
#define CCONS_VISITORS_H

//
// Defined utility visitor classes for operating on the clang AST.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include <string>
#include <vector>

#include <llvm/ADT/OwningPtr.h>

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/DeclGroup.h>

namespace clang {
	class SourceManager;
} // namespace clang

namespace ccons {

class StmtFinder : public clang::StmtVisitor<StmtFinder> {

public:

	StmtFinder(unsigned pos, const clang::SourceManager& sm);
	~StmtFinder();

	void VisitChildren(clang::Stmt *S);
	void VisitStmt(clang::Stmt *S);
	clang::Stmt * getStmt() const;

private:

	unsigned _pos;
	const clang::SourceManager& _sm;
	clang::Stmt *_S;

};

class StmtSplitter : public clang::StmtVisitor<StmtSplitter> {

public:

	StmtSplitter(const std::string& src,
	             const clang::SourceManager& sm,
	             const clang::LangOptions& options,
							 std::vector<clang::Stmt*> *stmts);
	~StmtSplitter();

	void VisitChildren(clang::Stmt *S);
	void VisitStmt(clang::Stmt *S);

private:

	const std::string& _src;
	const clang::SourceManager& _sm;
	const clang::LangOptions& _options;
	std::vector<clang::Stmt*> *_stmts;

};

template <typename T>
class FunctionBodyConsumer : public clang::ASTConsumer {

private:

	T *_SF;
	std::string _funcName;

public:

	explicit FunctionBodyConsumer<T>(T *SF, const char *funcName)
		: _SF(SF), _funcName(funcName) {}
	~FunctionBodyConsumer<T>() {}

	void HandleTopLevelDecl(clang::DeclGroupRef D) {
		for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
			if (clang::FunctionDecl *FD = dyn_cast<clang::FunctionDecl>(*I)) {
				if (FD->getNameAsCString() == _funcName) {
					if (clang::Stmt *S = FD->getBody()) {
						_SF->VisitChildren(S);
					}
				}
			}
		}
	}

};

} // namespace ccons

#endif // CCONS_VISITORS_H