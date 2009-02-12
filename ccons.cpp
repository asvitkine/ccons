#include <iostream>
#include <limits.h>
#include <histedit.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>

#include "Console.h"

using std::string;
using ccons::Console;

static llvm::cl::opt<bool>
	DebugMode("ccons-debug", llvm::cl::desc("Print debugging information"));

static Console * cc;

static const char *ccons_prompt(EditLine *e)
{
	return (cc ? cc->prompt() : "??? ");
}

int main(const int argc, char **argv)
{
	HistEvent event;

	llvm::cl::ParseCommandLineOptions(argc, argv, "ccons Interactive C Console\n");
	llvm::sys::PrintStackTraceOnErrorSignal();

	if (DebugMode)
		fprintf(stderr, "NOTE: Debugging information will be displayed.\n");

	History *h = history_init();
	history(h, &event, H_SETSIZE, INT_MAX);

	EditLine *e = el_init("ccons", stdin, stdout, stderr);
	el_set(e, EL_PROMPT, ccons_prompt);
	el_set(e, EL_EDITOR, "emacs");
	el_set(e, EL_HIST, history, h);

	Console console(DebugMode);
	cc = &console;
	const char *line = el_gets(e, NULL);
	while (line) {
		console.process(line);
		history(h, &event, H_ENTER, line);
		// FIXME: el_push() is broken... the second parameter should be const char *
		el_push(e, const_cast<char*>(console.input()));
		line = el_gets(e, NULL);
	}
	el_end(e);
	history_end(h);

	return 0;
}

