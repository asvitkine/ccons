#ifndef CCONS_REMOTE_CONSOLE_H
#define CCONS_REMOTE_CONSOLE_H

#include <stdio.h>
#include <string>

#include "Console.h"

namespace ccons {

class RemoteConsole : public IConsole {

public:

	RemoteConsole();
	virtual ~RemoteConsole();

	const char * prompt() const;
	const char * input() const;
	void process(const char *line);

private:

	FILE *_stream;
	std::string _prompt;
	std::string _input;

};

} // namespace ccons

#endif // CCONS_REMOTE_CONSOLE_H
