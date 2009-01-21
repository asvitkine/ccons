#ifndef CCONS_CONSOLE_H
#define CCONS_CONSOLE_H

#include <string>
#include <vector>
#include <algorithm>

#include <llvm/ADT/OwningPtr.h>

#include "Parser.h"

namespace llvm {
	class Linker;
	class ExecutionEngine;
} // namespace llvm

namespace clang {
	class Preprocessor;
	class Stmt;
	class Expr;
} // namespace clang

namespace ccons {

class Console {

public:

	Console();
	virtual ~Console();

	const char * prompt() const;
	void process(const char * line);

private:

	enum LineType {
		StmtLine,
		DeclLine,
		PrprLine,
	};
	
	typedef std::pair<std::string, LineType> CodeLine;

	std::string genAppendix(const char *line,
	                        std::string * fName,
	                        std::string * retType,
	                        std::vector<CodeLine> * moreLines);
	std::string genSource(std::string appendix);
	std::string genFunc(std::string line, std::string fname, std::string type);
	clang::Stmt * lineToStmt(std::string line,
	                         clang::SourceManager * sm,
													 std::string * src);
	bool getExprType(const clang::Expr *E, std::string * type);

	llvm::OwningPtr<llvm::Linker> _linker;
	llvm::OwningPtr<llvm::ExecutionEngine> _engine;
	llvm::OwningPtr<clang::Preprocessor> _pp;
	std::vector<CodeLine> _lines;
	std::string _prompt;
	clang::LangOptions _options;
	Parser _parser;

};

} // namespace ccons

#endif // CCONS_CONSOLE_H
