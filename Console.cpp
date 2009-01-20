#include "Console.h"

#include <iostream>
#include <map>
#include <vector>
#include <sstream>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Linker.h>
#include <llvm/Module.h>
#include <llvm/ModuleProvider.h>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

#include <clang/AST/AST.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Driver/CompileOptions.h>
#include <clang/Driver/TextDiagnosticPrinter.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/Lex/Preprocessor.h>


template <class T>
inline std::string to_string(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

using std::string;

namespace ccons {

class StmtFinder : public clang::StmtVisitor<StmtFinder> {
public:

	explicit StmtFinder(unsigned pos, const clang::SourceManager& sm) :
		_pos(pos), _sm(sm), _S(NULL) {}
	~StmtFinder() {}

	void VisitChildren(clang::Stmt *S) {
		for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end();
		     I != E; ++I) {
			if (*I) {
				Visit(*I);
			}
		}
	}

	void VisitStmt(clang::Stmt *S) {
		clang::SourceLocation Loc = S->getLocStart();
		if (Loc.isFileID() && _sm.getDecomposedFileLoc(Loc).second == _pos) {
			_S = S;
		}
	}

	clang::Stmt *getStmt() { return _S; }

private:
	unsigned _pos;
	const clang::SourceManager& _sm;
	clang::Stmt *_S;
};

class FunctionBodyConsumer : public clang::ASTConsumer {
public:

	explicit FunctionBodyConsumer(StmtFinder *SF) : SF(SF) {}
	~FunctionBodyConsumer() {}

	void HandleTopLevelDecl(clang::Decl *D) {
		if (clang::FunctionDecl *FD = dyn_cast<clang::FunctionDecl>(D)) {
			if (clang::Stmt *S = FD->getBody()) {
				SF->VisitChildren(S);
			}
		}
	}

private:
	StmtFinder *SF;
};


Console::Console() :
	_parser(_options),
	_prompt(">>> ")
{
	_options.C99 = true;
}

Console::~Console()
{
}

const char * Console::prompt() const
{
	return _prompt.c_str();
}

class ProxyDiagnosticClient : public clang::DiagnosticClient {
public:
	ProxyDiagnosticClient(clang::DiagnosticClient *DC) : _DC(DC) {}

	void HandleDiagnostic(clang::Diagnostic::Level DiagLevel,
	                      const clang::DiagnosticInfo &Info) {
		if (!_DC) return;
		if (Info.getID() != clang::diag::warn_unused_expr &&
		    Info.getID() != clang::diag::pp_macro_not_used)
			_DC->HandleDiagnostic(DiagLevel, Info);
	}

private:
	clang::DiagnosticClient *_DC;
};

string formatForType(string type)
{
	std::map<string, string> formatMap;
	formatMap[string("char")] = string("'%c'"); // or %hii if its not printable?
	formatMap[string("short")] = string("%hi");
	formatMap[string("unsigned short")] = string("%hu");
	formatMap[string("int")] = string("%i");
	formatMap[string("unsigned int")] = string("%u");
	formatMap[string("long")] = string("%li");
	formatMap[string("unsigned long")] = string("%lu");
	formatMap[string("long long")] = string("%lli");
	formatMap[string("unsigned long long")] = string("%llu");
	formatMap[string("float")] = string("%f");
	formatMap[string("double")] = string("%lf");
	formatMap[string("long double")] = string("%Lf");
	formatMap[string("size_t")] = string("%z");

	string format;
	// we also need a regexp library to parse input before giving it to clang...
	// for example, #include statements
	if (!strncmp(type.c_str(), "char [", 5) && type[type.length() - 1] == ']') {
		// string constants - this is a hack! we should support arrays better with
		// a special case for arrays of characters... (which we should call our own
		// smart function to print... that decides if its a string or not)
		format = "\\\"%s\\\"";
	} else if (type.length() > 0 && type[type.length() - 1] == '*') {
		format = "%p";
	} else if (formatMap.find(type) != formatMap.end()) {
		format = formatMap[type];
	}

	return format;
}

string Console::genSource()
{
	string src;
	for (unsigned i = 0; i < _lines.size(); ++i) {
		if (_lines[i].second == PrprLine) {
			src += _lines[i].first;
			src += "\n";
		} else if (_lines[i].second == DeclLine) {
			src += "extern ";
			src += _lines[i].first;
			src += "\n";
		}
	}
	return src;
}

string Console::genFunc(string line, string fname, string type)
{
	string func;
	func += type + " " + fname + "(void) {\n";
	if (type != "void")
		func += "return ";
	func += line;
	func += "\n}\n";
	return func;
}

clang::Stmt * Console::lineToStmt(std::string src, std::string line)
{
	// this may fail... if for ex. the line is an include
	src += "void doStuff() {\n";
	const unsigned pos = src.length();
	src += line;
	src += "\n}\n";

	ProxyDiagnosticClient pdc(NULL);
	clang::Diagnostic diag(&pdc);
	diag.setSuppressSystemWarnings(true);

	clang::SourceManager sm;
	StmtFinder finder(pos, sm);
	FunctionBodyConsumer consumer(&finder);
	_pp.reset(_parser.parse(src, &sm, &diag, &consumer));

	return finder.getStmt();
}

bool Console::getExprType(const clang::Expr *E, string *type)
{
	bool success = false;
	string exprType = E->getType().getAsString();
	string format = formatForType(exprType);
	if (!format.empty()) {
		*type = exprType;
		if (format == "\\\"%s\\\"") {
			*type = "char *";
		}
		success = true;
	}
	return success;
}

void printGV(const llvm::Function *F, const llvm::GenericValue &AV, string type)
{
	string format = formatForType(type);
	if (format.empty()) {
		std::cout << "=> (" << type << ")\n";
		return;
	}

	const llvm::FunctionType *FTy = F->getFunctionType();
	const llvm::Type *RetTy = FTy->getReturnType();
	switch (RetTy->getTypeID()) {
		case llvm::Type::IntegerTyID:
			printf(("=> (" + type + ") %lu\n").c_str(), AV.IntVal.getZExtValue());
			return;
		case llvm::Type::FloatTyID:
			printf(("=> (" + type + ") %f\n").c_str(), AV.FloatVal);
			return;
		case llvm::Type::DoubleTyID:
			printf(("=> (" + type + ") %lf\n").c_str(), AV.DoubleVal);
			return;
		case llvm::Type::PointerTyID:
			void *p = GVTOP(AV);
			if (p && type == "char *")
				printf(("=> (" + type + ") \"%s\"\n").c_str(), p);
			else
				printf(("=> (" + type + ") %p\n").c_str(), p);
			return;
		default: break;
	}

	assert(0 && "Unknown return type!");
}


void Console::process(const char * line)
{
	LineType type;

	while (isspace(*line)) line++;

	if (*line == '#') {
		type = PrprLine;
	} else {
		type = StmtLine;
	}

	string src = genSource();
	string ret_type = "void";
	if (const clang::Stmt *S = lineToStmt(src, line)) {
		if (const clang::Expr *E = dyn_cast<clang::Expr>(S)) {
			getExprType(E, &ret_type);
		} else if (const clang::DeclStmt *DS = dyn_cast<clang::DeclStmt>(S)) {
			type = DeclLine;
		}
	}

	string fname;
	if (type == StmtLine) {
		int funcNo = 0;
		for (unsigned i = 0; i < _lines.size(); ++i)
			funcNo += (_lines[i].second == StmtLine);
		fname = "__ccons_anon" + to_string(funcNo);
		string func = genFunc(line, fname, ret_type);
		src += func;
	} else if (type == DeclLine) {
		src += line;
		src += "\n";
	}

	_lines.push_back(make_pair(string(line), type));

	clang::TextDiagnosticPrinter tdp(llvm::errs());
	ProxyDiagnosticClient pdc(&tdp);
	clang::Diagnostic diag(&pdc);
	diag.setSuppressSystemWarnings(true);

	llvm::OwningPtr<clang::CodeGenerator> codegen;
	codegen.reset(CreateLLVMCodeGen(diag, _options, "-", false));
	clang::SourceManager sm;
	_pp.reset(_parser.parse(src, &sm, &diag, codegen.get()));
	llvm::Module *module = codegen->ReleaseModule();
	if (module) {
		if (!_linker)
			_linker.reset(new llvm::Linker("ccons", "ccons"));
		string err;
		_linker->LinkInModule(module, &err);
		std::cout << err;
		// link it with the existing ones
		if (type == StmtLine) {
			module = _linker->getModule();
			if (!_engine)
				_engine.reset(llvm::ExecutionEngine::create(module));
			llvm::Function *F = module->getFunction(fname.c_str());
			assert(F && "Function was not found!");
			std::vector<llvm::GenericValue> params;
			llvm::GenericValue result = _engine->runFunction(F, params);
			printGV(F, result, ret_type);
		}
	}

	_pp.reset();
}

} // namespace ccons
