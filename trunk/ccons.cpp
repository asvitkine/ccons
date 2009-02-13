#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>

#include "Console.h"
#include "EditLineReader.h"

using std::string;
using ccons::Console;
using ccons::LineReader;
using ccons::EditLineReader;
using ccons::StdInLineReader;

static llvm::cl::opt<bool>
	DebugMode("ccons-debug", llvm::cl::desc("Print debugging information"));
static llvm::cl::opt<bool>
	UseStdIo("use-std-io", llvm::cl::desc("Use standard IO for input and output"));
static llvm::cl::opt<bool>
	MultiProcess("multi-process", llvm::cl::desc("Run in multi-process mode"));

static void *pipe_input(void *ptr)
{
	FILE *ps = (FILE *) ptr;
	char buf[1024];
	while (1) {
		const char *s = fgets(buf, sizeof(buf), ps);
		if (s) printf("%s", buf);
	}
	return NULL;
}

int main(const int argc, char **argv)
{
	llvm::cl::ParseCommandLineOptions(argc, argv, "ccons Interactive C Console\n");
	llvm::sys::PrintStackTraceOnErrorSignal();

	if (DebugMode)
		fprintf(stderr, "NOTE: Debugging information will be displayed.\n");

	Console console(DebugMode);
	llvm::OwningPtr<LineReader> reader;

	FILE *ps;
	pthread_t reader_thread;

	if (MultiProcess) {
		ps = popen("./ccons --use-std-io", "r+");
		assert(ps);
		fcntl(fileno(ps), F_SETFL, O_NONBLOCK);
		pthread_create(&reader_thread, NULL, pipe_input, (void *) ps);
	}

	if (UseStdIo)
		reader.reset(new StdInLineReader);
	else
		reader.reset(new EditLineReader);

	const char *line = reader->readLine(console.prompt(), console.input());
	while (line) {
		if (MultiProcess) {
			fputs(line, ps);
			usleep(100000);
		} else {
			console.process(line);
		}
		line = reader->readLine(console.prompt(), console.input());
	}

	if (MultiProcess)
		pclose(ps);

	return 0;
}

