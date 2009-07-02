#ifndef CCONS_VISITORS_H
#define CCONS_VISITORS_H

//
// Defines utility visitor classes for operating on the clang AST.
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
	class ASTContext;
	class SourceManager;
} // namespace clang

namespace ccons {

// StmtFinder will attempt to find a clang Stmt at the specified
// offset in the source (pos).
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

// StmtSplitter will extract clang Stmts from the specified source.
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

// ASTConsumer that visits function body Stmts and passes
// those to a specific StmtVisitor.
template <typename T>
class FunctionBodyConsumer : public clang::ASTConsumer {

private:

	T *_SV;
	std::string _funcName;

public:

	FunctionBodyConsumer<T>(T *SV, const char *funcName)
		: _SV(SV), _funcName(funcName) {}
	~FunctionBodyConsumer<T>() {}

	void HandleTopLevelDecl(clang::DeclGroupRef D) {
		for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
			if (clang::FunctionDecl *FD = dyn_cast<clang::FunctionDecl>(*I)) {
				if (FD->getNameAsCString() == _funcName) {
					if (clang::Stmt *S = FD->getBody()) {
						_SV->VisitChildren(S);
					}
				}
			}
		}
	}

};

} // namespace ccons

#endif // CCONS_VISITORS_H