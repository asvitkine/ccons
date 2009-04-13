#ifndef CCONS_REMOTE_CONSOLE_H
#define CCONS_REMOTE_CONSOLE_H

#include <stdio.h>
#include <string>
#include <sstream>

#include "Console.h"

namespace ccons {

class RemoteConsole : public IConsole {

public:

	explicit RemoteConsole(bool DebugMode);
	virtual ~RemoteConsole();

	const char * prompt() const;
	const char * input() const;
	void process(const char *line);

private:

	bool _DebugMode;
	FILE *_ostream;
	FILE *_istream;
	std::string _prompt;
	std::string _input;

	void cleanup();
	void reset();

};



class SerializedOutputConsole: public IConsole {

public:

	explicit SerializedOutputConsole(bool DebugMode);
	virtual ~SerializedOutputConsole();

	const char * prompt() const;
	const char * input() const;
	void process(const char *line);

private:

	llvm::OwningPtr<IConsole> _console;
	std::stringstream _ss_out;
	std::stringstream _ss_err;

};

} // namespace ccons

#endif // CCONS_REMOTE_CONSOLE_H
