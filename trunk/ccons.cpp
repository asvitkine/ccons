#include <iostream>
#include <editline/readline.h>

#include "Console.h"

using std::string;
using ccons::Console;

char **ccons_completion(const char *text, int start, int end)
{
	// TODO call console.complete(...)
	return NULL;
}

int main(const int argc, const char **argv)
{
	rl_readline_name = "ccons";
	rl_attempted_completion_function = ccons_completion;

	Console console;
	char *line = readline(console.prompt());
	while (line) {
		console.process(line);
		line = readline(console.prompt());
	}

	return 0;
}

