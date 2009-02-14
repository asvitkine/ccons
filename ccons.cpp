#include <iostream>
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <fcntl.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>

#include "ccons.pb.h"

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
using ccons::ConsoleOutput;

static llvm::cl::opt<bool>
	DebugMode("ccons-debug", llvm::cl::desc("Print debugging information"));
static llvm::cl::opt<bool>
	UseStdIo("use-std-io", llvm::cl::desc("Use standard IO for input and output"));
static llvm::cl::opt<bool>
	MultiProcess("multi-process", llvm::cl::desc("Run in multi-process mode"));
static llvm::cl::opt<bool>
	ProtoBufOutput("proto-buf-output", llvm::cl::desc("Output as protobuf"));

static std::stringstream ss_out;
static std::stringstream ss_err;

static IConsole * createConsole()
{
	if (MultiProcess)
		return new RemoteConsole;
	else if (ProtoBufOutput)
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

int main(const int argc, char **argv)
{
	llvm::cl::ParseCommandLineOptions(argc, argv, "ccons Interactive C Console\n");
	llvm::sys::PrintStackTraceOnErrorSignal();

	if (DebugMode)
		fprintf(stderr, "NOTE: Debugging information will be displayed.\n");

	llvm::OwningPtr<IConsole> console(createConsole());
	llvm::OwningPtr<LineReader> reader(createReader());

	const char *line = reader->readLine(console->prompt(), console->input());
	while (line) {
		console->process(line);
		if (ProtoBufOutput) {
			ConsoleOutput output;
			output.set_output(ss_out.str());
			output.set_error_output(ss_err.str());
			output.set_prompt(console->prompt());
			output.set_input(console->input());
			std::cout << output.DebugString();
			std::cout.flush();
			ss_out.clear();
			ss_err.clear();
		}
		line = reader->readLine(console->prompt(), console->input());
	}

	return 0;
}

