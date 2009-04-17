#include <iostream>
#include <fstream>
#include <sstream>

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
using ccons::SerializedOutputConsole;
using ccons::LineReader;
using ccons::EditLineReader;
using ccons::StdInLineReader;

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
		return new RemoteConsole(DebugMode);
	else if (SerializedOutput)
		return new SerializedOutputConsole(DebugMode);		
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

int main(const int argc, char **argv)
{
	llvm::cl::ParseCommandLineOptions(argc, argv, "ccons Interactive C Console\n",
	                                  false/*, "ccons-"*/);
	// TODO SetVersionPrinter()

	if (DebugMode && !SerializedOutput) {
		std::cerr << "NOTE: Debugging information will be displayed.\n";
		llvm::sys::PrintStackTraceOnErrorSignal();
	}

	llvm::OwningPtr<IConsole> console(createConsole());
	llvm::OwningPtr<LineReader> reader(createReader());

	const char *line = reader->readLine(console->prompt(), console->input());
	while (line) {
		console->process(line);
		line = reader->readLine(console->prompt(), console->input());
	}

	return 0;
}

