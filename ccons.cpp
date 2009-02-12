#include <iostream>

#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>

#include "Console.h"
#include "EditLineReader.h"

using std::string;
using ccons::Console;
using ccons::EditLineReader;

static llvm::cl::opt<bool>
	DebugMode("ccons-debug", llvm::cl::desc("Print debugging information"));


int main(const int argc, char **argv)
{
	llvm::cl::ParseCommandLineOptions(argc, argv, "ccons Interactive C Console\n");
	llvm::sys::PrintStackTraceOnErrorSignal();

	if (DebugMode)
		fprintf(stderr, "NOTE: Debugging information will be displayed.\n");

	Console console(DebugMode);

	EditLineReader reader;
	const char *line = reader.readLine(console.prompt(), console.input());
	while (line) {
		console.process(line);
		line = reader.readLine(console.prompt(), console.input());
	}

	return 0;
}

