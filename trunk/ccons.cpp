#include <iostream>
#include <fstream>
#include <sstream>

#include <signal.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>

#include "Console.h"
#include "RemoteConsole.h"
#include "EditLineReader.h"

using std::string;
using ccons::Console;
using ccons::IConsole;
using ccons::RemoteConsole;
using ccons::LineReader;
using ccons::EditLineReader;
using ccons::StdInLineReader;
using ccons::SerializedConsoleOutput;

static llvm::cl::opt<bool>
	DebugMode("ccons-debug",
						llvm::cl::desc("Print debugging information"));
static llvm::cl::opt<bool>
	UseStdIo("ccons-use-std-io",
	         llvm::cl::desc("Use standard IO for input and output"));
static llvm::cl::opt<bool>
	SerializedOutput("ccons-serialized-output",
	                 llvm::cl::desc("Output will be serialized"));
static llvm::cl::opt<bool>
	MultiProcess("ccons-multi-process",
							 llvm::cl::desc("Run in multi-process mode"));

static std::stringstream ss_out;
static std::stringstream ss_err;

static IConsole * createConsole()
{
	if (MultiProcess)
		return new RemoteConsole;
	else if (SerializedOutput)
		return new Console(DebugMode, ss_out, ss_err);		
	else
		return new Console(DebugMode);
}

static LineReader * createReader()
{
	if (UseStdIo)
		return new StdInLineReader;
	else
		return new EditLineReader;
}

void gotsig(int signo)
{
	string str;

	if (signo == SIGBUS) {
		str = "Bus error\n";
	} else if (signo == SIGSEGV) {
		str = "Segmentation fault\n";
	} else if (signo == SIGABRT) {
		str = "Abort trap\n";
	}

	SerializedConsoleOutput sco("", str, "", "");
	sco.writeToString(&str);
	std::cout << str;
	std::cout.flush();
	exit(-1);
}

void goodbye(void)
{
	string str;
	SerializedConsoleOutput sco("", "", "", "");
	sco.writeToString(&str);
	std::cout << str;
	std::cout.flush();
}

int main(const int argc, char **argv)
{
	llvm::cl::ParseCommandLineOptions(argc, argv, "ccons Interactive C Console\n",
	                                  false/*, "ccons-"*/);
	// TODO SetVersionPrinter()

/*
	TODO:
	Loading of external lib:
	#include <llvm/System/DynamicLibrary.h>
	char *libz = "/usr/lib/libz.dylib";
	llvm::sys::Path path(libz);
	if (path.isDynamicLibrary()) {
		std::string errMsg;
		llvm::sys::DynamicLibrary::LoadLibraryPermanently(libz, &errMsg);
	}
*/

	if (DebugMode) {
		std::cerr << "NOTE: Debugging information will be displayed.\n";
		llvm::sys::PrintStackTraceOnErrorSignal();
	} else if (SerializedOutput) {
		signal(SIGBUS, gotsig);
		signal(SIGSEGV, gotsig);
		signal(SIGABRT, gotsig);
		atexit(goodbye);
	} else if (MultiProcess) {
		signal(SIGCHLD, SIG_IGN);
	}

	llvm::OwningPtr<IConsole> console(createConsole());
	llvm::OwningPtr<LineReader> reader(createReader());

	const char *line = reader->readLine(console->prompt(), console->input());
	while (line) {
		console->process(line);
		if (SerializedOutput) {
			string str;
			SerializedConsoleOutput sco(ss_out.str(), ss_err.str(),
																	console->prompt(), console->input());
      sco.writeToString(&str);
			std::cout << str;
			std::cout.flush();
			ss_out.str("");
			ss_err.str("");
		}
		line = reader->readLine(console->prompt(), console->input());
	}

	return 0;
}

