#include <iostream>
#include <histedit.h>

#include "Console.h"

using std::string;
using ccons::Console;

static Console * cc;

static const char *ccons_prompt(EditLine *e)
{
	return (cc ? cc->prompt() : "??? ");
}

int main(const int argc, const char **argv)
{
	EditLine *e = el_init("ccons", stdin, stdout, stderr);
	el_set(e, EL_PROMPT, ccons_prompt);

	Console console;
	cc = &console;
	const char *line = el_gets(e, NULL);
	while (line) {
		console.process(line);
		// FIXME: el_push() is broken... the second parameter should be const char *
		el_push(e, const_cast<char*>(console.input()));
		line = el_gets(e, NULL);
	}
	el_end(e);

	return 0;
}

