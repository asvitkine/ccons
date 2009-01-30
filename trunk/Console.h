#ifndef CCONS_CONSOLE_H
#define CCONS_CONSOLE_H

#include <string>
#include <vector>
#include <algorithm>

#include <llvm/ADT/OwningPtr.h>

#include <clang/Basic/LangOptions.h>

namespace llvm {
	class ExecutionEngine;
	class Function;
	class GenericValue;
	class Linker;
} // namespace llvm

namespace clang {
	class DeclStmt;
	class Expr;
	class Preprocessor;
	class QualType;
	class SourceManager;
	class Stmt;
} // namespace clang

namespace ccons {

class Parser;

class Console {

public:

	Console();
	virtual ~Console();

	const char * prompt() const;
	const char * input() const;
	void process(const char *line);

private:

	typedef std::pair<unsigned, unsigned> SrcRange;

	enum LineType {
		StmtLine,
		DeclLine,
		PrprLine,
	};
	
	typedef std::pair<std::string, LineType> CodeLine;

	void printGV(const llvm::Function *F,
                     const llvm::GenericValue& GV,
                     const clang::QualType& QT);
	SrcRange getStmtRange(const clang::Stmt *S, const clang::SourceManager& sm);
	bool handleDeclStmt(const clang::DeclStmt *DS,
	                    const std::string& src,
	                    std::string *appendix,
	                    std::string *funcBody,
	                    std::vector<CodeLine> *moreLines,
	                    const clang::SourceManager& sm);
	std::string genAppendix(const char *line,
	                        std::string *fName,
	                        clang::QualType& QT,
	                        std::vector<CodeLine> *moreLines,
	                        bool *hadErrors);
	std::string genSource(std::string appendix);
	clang::Stmt * lineToStmt(std::string line,
	                         clang::SourceManager *sm,
	                         std::string *src);

	clang::LangOptions _options;
	llvm::OwningPtr<llvm::Linker> _linker;
	llvm::OwningPtr<llvm::ExecutionEngine> _engine;
	llvm::OwningPtr<Parser> _parser;
	std::vector<CodeLine> _lines;
	std::string _buffer;
	std::string _prompt;
	std::string _input;

};

} // namespace ccons

#endif // CCONS_CONSOLE_H
