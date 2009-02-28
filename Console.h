#ifndef CCONS_CONSOLE_H
#define CCONS_CONSOLE_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <llvm/Support/raw_ostream.h>
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

class IConsole {

public:

	virtual ~IConsole() {}
	virtual const char * prompt() const = 0;
	virtual const char * input() const = 0;
	virtual void process(const char *line) = 0;

};

class Console : public IConsole {

public:

	Console(bool _debugMode,
	        std::ostream& out = std::cout,
	        std::ostream& err = std::cerr);
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
	             const clang::QualType& QT) const;
	SrcRange getStmtRange(const clang::Stmt *S, const clang::SourceManager& sm) const;
	bool handleDeclStmt(const clang::DeclStmt *DS,
	                    const std::string& src,
	                    std::string *appendix,
	                    std::string *funcBody,
	                    std::vector<CodeLine> *moreLines,
	                    const clang::SourceManager& sm);
	std::string genAppendix(const char *source,
	                        const char *line,
	                        std::string *fName,
	                        clang::QualType& QT,
	                        std::vector<CodeLine> *moreLines,
	                        bool *hadErrors);
	std::string genSource(std::string appendix) const;
	clang::Stmt * lineToStmt(std::string line,
	                         clang::SourceManager *sm,
	                         std::string *src);

	bool _debugMode;
	std::ostream& _out;
	std::ostream& _err;
	mutable llvm::raw_os_ostream _raw_err;
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
