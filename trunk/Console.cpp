#include "Console.h"

#include <iostream>
#include <map>
#include <vector>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Module.h>
#include <llvm/ModuleProvider.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <clang/AST/AST.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Driver/CompileOptions.h>
#include <clang/Driver/TextDiagnosticPrinter.h>
#include <clang/CodeGen/ModuleBuilder.h>


using std::string;

namespace ccons {

class StmtFinder : public clang::StmtVisitor<StmtFinder> {
public:

	explicit StmtFinder(unsigned pos) : pos(pos), S(NULL) {}
	~StmtFinder() {}

	void VisitChildren(clang::Stmt *S) {
		for (clang::Stmt::child_iterator I = S->child_begin(), E = S->child_end(); I != E; ++I) {
			if (*I) {
				Visit(*I);
			}
		}
	}

	void VisitStmt(clang::Stmt *S) {
		if (clang::Expr *E = dyn_cast<clang::Expr>(S)) {
			if (E->getLocStart().getRawFilePos() == pos) {
				this->S = S;
			}
		}
	}

	clang::Stmt *getStmt() { return S; }

private:
	unsigned pos;
	clang::Stmt *S;
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
	ProxyDiagnosticClient(clang::DiagnosticClient *DC) : DC(DC) {}

	void HandleDiagnostic(clang::Diagnostic::Level DiagLevel, const clang::DiagnosticInfo &Info) {
		if (Info.getID() != clang::diag::warn_unused_expr)
			DC->HandleDiagnostic(DiagLevel, Info);
	}

private:
	clang::DiagnosticClient *DC;
};

string transform(string line, string type)
{
	std::map<string, string> type_to_format_string;
	type_to_format_string[string("char")] = string("'%c'"); // or %hii if its not printable?
	type_to_format_string[string("short")] = string("%hi");
	type_to_format_string[string("int")] = string("%i");
	type_to_format_string[string("long")] = string("%li");
	type_to_format_string[string("long long")] = string("%lli");
	type_to_format_string[string("float")] = string("%f");
	type_to_format_string[string("double")] = string("%lf");
	type_to_format_string[string("long double")] = string("%Lf");
	type_to_format_string[string("size_t")] = string("%z");

	string format_string;
	// we also need a regexp library to parse input before giving it to clang...
	// for example, #include statements
	if (!strncmp(type.c_str(), "char [", 5) && type[type.length() - 1] == ']') {
		// string constants - this is a hack! we should support arrays better with
		// a special case for arrays of characters... (which we should call our own
		// smart function to print... that decides if its a string or not)
		format_string = "\\\"%s\\\"";
	} else if (type.length() > 0 && type[type.length() - 1] == '*') {
		format_string = "%p";
	} else if (type_to_format_string.find(type) != type_to_format_string.end()) {
		format_string = type_to_format_string[type];
	}

	if (!format_string.empty()) {
		line = line.substr(0, line.length() - 1);
		line = "printf(\"=> (" + type + ") " + format_string + "\\n\"," + line + ");\n";
	} else {
		line.clear();
	}

	return line;
}

void Console::process(const char * line)
{
	string header = "int main(int argc, char *argv[])\n{\n";
	string footer = "return 0;\n}\n";

	const unsigned pos = header.length() + _body.length();
	string old_body = _body;
	_body += line;
	_body += "\n";

	string source = header + _body + footer;

	clang::TextDiagnosticPrinter tdp(llvm::errs());
	ProxyDiagnosticClient pdc(&tdp);
	clang::Diagnostic diag(&pdc);
	diag.setSuppressSystemWarnings(true);

	StmtFinder finder(pos);
	FunctionBodyConsumer consumer(&finder);
	_parser.parse(source, &diag, &consumer);
	if (clang::Stmt *S = finder.getStmt()) {
		if (clang::Expr *E = dyn_cast<clang::Expr>(S)) {
			string type = E->getType().getAsString();
			string format_string;
			string transformed_line = transform(line, type);
			if (!transformed_line.empty()) {
				string source2 = header + old_body + transformed_line + footer;
				llvm::OwningPtr<clang::CodeGenerator> codegen(CreateLLVMCodeGen(diag, _options, "-", false));
				_parser.parse(source2, &diag, codegen.get());
				llvm::Module *module = codegen->ReleaseModule();
				if (module) {
					// provider takes ownership of module
					llvm::ExistingModuleProvider *provider = new llvm::ExistingModuleProvider(module);
					// execution engine takes ownership of provider
					llvm::OwningPtr<llvm::ExecutionEngine> engine(llvm::ExecutionEngine::create(provider, /* ForceInterpreter = */ true));
					assert(engine && "Could not create llvm::ExecutionEngine!");
					std::vector<std::string> params;
					params.push_back(">>>");
					engine->runFunctionAsMain(module->getFunction("main"), params, 0);
				}
			} else {
				std::cout << "=> (" << type << ")\n";
			}
		}
	}
}

} // namespace ccons
