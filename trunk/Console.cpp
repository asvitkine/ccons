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
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompileOptions.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/Lex/Preprocessor.h>

#include "ClangUtils.h"
#include "Diagnostics.h"
#include "Parser.h"
#include "SrcGen.h"
#include "StringUtils.h"
#include "Visitors.h"

using std::string;

namespace ccons {

Console::Console(bool debugMode, std::ostream& out, std::ostream& err) :
	_debugMode(debugMode),
	_out(out),
	_err(err),
	_raw_err(err),
	_prompt(">>> ")
{
	_options.C99 = true;
	_parser.reset(new Parser(_options));
	// Declare exit() so users may call it without needing to #include <stdio.h>
	_lines.push_back(CodeLine("void exit(int status);", DeclLine));
}

Console::~Console()
{
	if (_linker)
		_linker->releaseModule();
}

const char * Console::prompt() const
{
	return _prompt.c_str();
}

const char * Console::input() const
{
	return _input.c_str();
}

string Console::genSource(string appendix) const
{
	string src;
	for (unsigned i = 0; i < _lines.size(); ++i) {
		if (_lines[i].second == PrprLine) {
			src += _lines[i].first;
			src += "\n";
		} else if (_lines[i].second == DeclLine) {
			src += _lines[i].first;
			src += "\n";
		}
	}
	src += appendix;
	return src;
}

int Console::splitInput(const string& source,
                        const string& input,
                        std::vector<string> *statements)
{
	string src = source;
	src += "void __ccons_internal() {\n";
	const unsigned pos = src.length();
	src += input;
	src += "\n}\n";

	// Set offset on the diagnostics provider.
	_dp->setOffset(pos);
	std::vector<clang::Stmt*> stmts;
	clang::SourceManager *sm = new clang::SourceManager;
	StmtSplitter splitter(src, *sm, _options, &stmts);
	FunctionBodyConsumer<StmtSplitter> consumer(&splitter, "__ccons_internal");
	if (_debugMode)
		oprintf(_err, "Parsing in splitInput()...\n");
	_parser->parse(src, _dp->getDiagnostic(), &consumer, sm);

	if (stmts.size() == 1) {
		statements->push_back(input);
	} else {
		for (unsigned i = 0; i < stmts.size(); i++) {
			SrcRange range = getStmtRangeWithSemicolon(stmts[i], *sm, _options);
			string s = src.substr(range.first, range.second - range.first);
			if (_debugMode)
				oprintf(_err, "Split %d is: %s\n", i, s.c_str());
			statements->push_back(s);
		}
	}

	return stmts.size();
}

clang::Stmt * Console::lineToStmt(const std::string& line,
                                  std::string *src)
{
	*src += "void __ccons_internal() {\n";
	const unsigned pos = src->length();
	*src += line;
	*src += "\n}\n";

	// Set offset on the diagnostics provider.
	_dp->setOffset(pos);
	clang::SourceManager *sm = new clang::SourceManager;
	StmtFinder finder(pos, *sm);
	FunctionBodyConsumer<StmtFinder> consumer(&finder, "__ccons_internal");
	if (_debugMode)
		oprintf(_err, "Parsing in lineToStmt()...\n");
	_parser->parse(*src, _dp->getDiagnostic(), &consumer, sm);

	if (_dp->getDiagnostic()->hasErrorOccurred()) {
		src->clear();
		return NULL;
	}

	return finder.getStmt();
}

void Console::printGV(const llvm::Function *F,
                      const llvm::GenericValue& GV,
                      const clang::QualType& QT) const
{
	const char *type = QT.getAsString().c_str();
	const llvm::FunctionType *FTy = F->getFunctionType();
	const llvm::Type *RetTy = FTy->getReturnType();
	switch (RetTy->getTypeID()) {
		case llvm::Type::IntegerTyID:
			if (QT->isUnsignedIntegerType())
				oprintf(_out, "=> (%s) %lu\n", type, GV.IntVal.getZExtValue());
			else
				oprintf(_out, "=> (%s) %ld\n", type, GV.IntVal.getZExtValue());
			return;
		case llvm::Type::FloatTyID:
			oprintf(_out, "=> (%s) %f\n", type, GV.FloatVal);
			return;
		case llvm::Type::DoubleTyID:
			oprintf(_out, "=> (%s) %lf\n", type, GV.DoubleVal);
			return;
		case llvm::Type::PointerTyID: {
			void *p = GVTOP(GV);
			// FIXME: this is a hack
			if (p && !strncmp(type, "char", 4)) {
				oprintf(_out, "=> (%s) \"%s\"\n", type, p);
			} else if (QT->isFunctionType()) {
				string str = "*";
				QT.getUnqualifiedType().getAsStringInternal(str);
				type = str.c_str();
				oprintf(_out, "=> (%s) %p\n", type, p);
			} else {
				oprintf(_out, "=> (%s) %p\n", type, p);
			}
			return;
		}
		case llvm::Type::VoidTyID:
			if (strcmp(type, "void"))
				oprintf(_out, "=> (%s)\n", type);
			return;
		default:
			break;
	}

	assert(0 && "Unknown return type!");
}

bool Console::handleDeclStmt(const clang::DeclStmt *DS,
                             const string& src,
                             string *appendix,
                             string *funcBody,
                             std::vector<CodeLine> *moreLines)
{
	bool initializers = false;
	for (clang::DeclStmt::const_decl_iterator D = DS->decl_begin(),
			 E = DS->decl_end(); D != E; ++D) {
		if (const clang::VarDecl *VD = dyn_cast<clang::VarDecl>(*D)) {
			if (VD->getInit()) {
				initializers = true;
			}
		}
	}
	if (initializers) {
		std::vector<string> decls;
		std::vector<string> stmts;
		clang::SourceManager *sm = _parser->getLastParseOperation()->getSourceManager();
		clang::ASTContext *context = _parser->getLastParseOperation()->getASTContext();
		for (clang::DeclStmt::const_decl_iterator D = DS->decl_begin(),
				 E = DS->decl_end(); D != E; ++D) {
			if (const clang::VarDecl *VD = dyn_cast<clang::VarDecl>(*D)) {
				string decl = genVarDecl(VD->getType(), VD->getNameAsCString());
				if (const clang::Expr *I = VD->getInit()) {
					SrcRange range = getStmtRange(I, *sm, _options);
					if (I->isConstantInitializer(*context)) {
						// Keep the whole thing in the global context.
						std::stringstream global;
						global << decl << " = ";
						global << src.substr(range.first, range.second - range.first);
						*appendix += global.str() + ";\n";
					} else if (const clang::InitListExpr *ILE = dyn_cast<clang::InitListExpr>(I)) {
						// If it's an InitListExpr like {'a','b','c'}, but with non-constant
						// initializers, then split it up into x[0] = 'a'; x[1] = 'b'; and
						// so forth, which would go in the function body, while making the
						// declaration global.
						unsigned numInits = ILE->getNumInits();
						for (unsigned i = 0; i < numInits; i++) {
							std::stringstream stmt;
							stmt << VD->getNameAsCString() << "[" << i << "] = ";
							range = getStmtRange(ILE->getInit(i), *sm, _options);
							stmt << src.substr(range.first, range.second - range.first) << ";";
							stmts.push_back(stmt.str());
						}
						*appendix += decl + ";\n";
					} else {
						std::stringstream stmt;
						stmt << VD->getNameAsCString() << " = "
						     << src.substr(range.first, range.second - range.first) << ";";
						stmts.push_back(stmt.str());
						*appendix += decl + ";\n";
					}
				} else {
					// Just add it as a definition without an initializer.
					*appendix += decl + ";\n";
				}
				decls.push_back(decl + ";");
			}
		}
		for (unsigned i = 0; i < decls.size(); ++i)
			moreLines->push_back(CodeLine("extern " + decls[i], DeclLine));
		for (unsigned i = 0; i < stmts.size(); ++i) {
			moreLines->push_back(CodeLine(stmts[i], StmtLine));
			*funcBody += stmts[i] + "\n";
		}
		return true;
	}
	return false;
}

string Console::genAppendix(const char *source,
                            const char *line,
                            string *fName,
                            clang::QualType& QT,
                            std::vector<CodeLine> *moreLines,
                            bool *hadErrors)
{
	bool wasExpr = false;
	string appendix;
	string funcBody;
	string src = source;

	while (isspace(*line)) line++;

	*hadErrors = false;
	if (*line == '#') {
		moreLines->push_back(CodeLine(line, PrprLine));
		appendix += line;
	} else if (const clang::Stmt *S = lineToStmt(line, &src)) {
		if (_debugMode)
			oprintf(_err, "Found Stmt for input.\n");
		if (const clang::Expr *E = dyn_cast<clang::Expr>(S)) {
			QT = E->getType();
			funcBody = line;
			moreLines->push_back(CodeLine(line, StmtLine));
			wasExpr = true;
		} else if (const clang::DeclStmt *DS = dyn_cast<clang::DeclStmt>(S)) {
			if (!handleDeclStmt(DS, src, &appendix, &funcBody, moreLines)) {
				moreLines->push_back(CodeLine(line, DeclLine));
				appendix += line;
				appendix += "\n";
			}
		} else {
			funcBody = line; // ex: if statement
			moreLines->push_back(CodeLine(line, StmtLine));
		}
	} else if (src.empty()) {
		_err << "\nNote: Last line ignored due to errors.\n";
		*hadErrors = true;
	}

	if (!funcBody.empty()) {
		int funcNo = 0;
		for (unsigned i = 0; i < _lines.size(); ++i)
			funcNo += (_lines[i].second == StmtLine);
		*fName = "__ccons_anon" + to_string(funcNo);
		int bodyOffset;
		clang::ASTContext *context = _parser->getLastParseOperation()->getASTContext();
		appendix += genFunc(wasExpr ? &QT : NULL, context, *fName, funcBody, bodyOffset);
		_dp->setOffset(bodyOffset + strlen(source));
		if (_debugMode)
			oprintf(_err, "Generating function %s()...\n", fName->c_str());
	}

	return appendix;
}

void Console::process(const char *line)
{
	std::vector<CodeLine> linesToAppend;
	bool hadErrors = false;
	string appendix;
	string src = genSource("");	

	_buffer += line;
	Parser p(_options);
	int indentLevel;
	const clang::FunctionDecl *FD;
	bool shouldBeTopLevel = false;
	switch (p.analyzeInput(src, _buffer, indentLevel, FD)) {
		case Parser::Incomplete:
			_input = string(indentLevel * 2, ' ');
			_prompt = "... ";
			return;
		case Parser::TopLevel:
			shouldBeTopLevel = true;
		case Parser::Stmt:
			_prompt = ">>> ";
			_input = "";
			break;
	}

	_dp.reset(new DiagnosticsProvider(_raw_err, _options));

	if (shouldBeTopLevel) {
		if (_debugMode)
			oprintf(_err, "Treating input as top-level.\n");
		appendix = _buffer;
		linesToAppend.push_back(CodeLine(getFunctionDeclAsString(FD), DeclLine));
		_buffer.clear();

		if (hadErrors)
			return;

		src = genSource(appendix);
		string empty;
		clang::QualType retType(0, 0);
		if (compileLinkAndRun(src, empty, retType)) {
			for (unsigned i = 0; i < linesToAppend.size(); ++i)
				_lines.push_back(linesToAppend[i]);
		}
	} else {
		if (_debugMode)
			oprintf(_err, "Treating input as function-level.\n");
		std::vector<string> split;
		if (_buffer[0] == '#') {
			split.push_back(_buffer);
		} else {
			string input = _buffer;
			splitInput(src, input, &split);
		}
		_buffer.clear();

		for (unsigned i = 0; i < split.size(); i++) {
			string fName;
			clang::QualType retType(0, 0);
			appendix = genAppendix(src.c_str(), split[i].c_str(), &fName, retType, &linesToAppend, &hadErrors);
			if (hadErrors)
				return;
			src = genSource(appendix);

			if (compileLinkAndRun(src, fName, retType)) {
				for (unsigned i = 0; i < linesToAppend.size(); ++i)
					_lines.push_back(linesToAppend[i]);
				}
		}
	}
	_parser->releaseAccumulatedParseOperations();
}

bool Console::compileLinkAndRun(const string& src,
                                const string& fName,
                                const clang::QualType& retType)
{
	if (_debugMode)
		oprintf(_err, "Running code-generator.\n");

	llvm::OwningPtr<clang::CodeGenerator> codegen;
	clang::CompileOptions compileOptions;
	codegen.reset(CreateLLVMCodeGen(*_dp->getDiagnostic(), "-", compileOptions));
	Parser p2(_options); // we keep the other parser around because of QT...
	if (_debugMode)
		oprintf(_err, "Parsing in compileLinkAndRun()...\n");
	p2.parse(src, _dp->getDiagnostic(), codegen.get());
	if (_dp->getDiagnostic()->hasErrorOccurred()) {
		_err << "\nNote: Last line ignored due to errors.\n";
		return false;
	}

	llvm::Module *module = codegen->ReleaseModule();
	if (module) {
		if (!_linker)
			_linker.reset(new llvm::Linker("ccons", "ccons"));
		string error;
		_linker->LinkInModule(module, &error);
		if (!error.empty())
			oprintf(_err, "Error: %s\n", error.c_str());
		// link it with the existing ones
		if (!fName.empty()) {
			module = _linker->getModule();
			if (!_engine)
				_engine.reset(llvm::ExecutionEngine::create(module));
			llvm::Function *F = module->getFunction(fName.c_str());
			assert(F && "Function was not found!");
			std::vector<llvm::GenericValue> params;
			if (_debugMode)
				oprintf(_err, "Calling function %s()...\n", fName.c_str());
			llvm::GenericValue result = _engine->runFunction(F, params);
			if (retType.getTypePtr())
				printGV(F, result, retType);
		} else {
			if (_debugMode)
				oprintf(_err, "Code generation done; function call not needed.\n");
		}
	} else {
		if (_debugMode)
			oprintf(_err, "Could not release module.\n");
	}

	return true;
}


} // namespace ccons
